#include "server/websocket_server.h"
#include "security/jwt.h"
#include "config/env.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <regex>

namespace sohbet {
namespace server {

// WebSocket GUID for handshake
static const std::string WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Base64 encoding helper
static std::string base64_encode(const unsigned char* input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return result;
}

// WebSocketConnection implementation
bool WebSocketConnection::sendMessage(const std::string& message) {
    ssize_t sent = send(socket_fd_, message.c_str(), message.length(), 0);
    return sent > 0;
}

// WebSocketServer implementation
WebSocketServer::WebSocketServer(int port)
    : port_(port), server_socket_(-1), running_(false) {
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::initializeSocket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Failed to create WebSocket server socket" << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR on WebSocket server" << std::endl;
        close(server_socket_);
        return false;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind WebSocket server to port " << port_ << std::endl;
        close(server_socket_);
        return false;
    }
    
    // Listen for connections
    if (listen(server_socket_, 10) < 0) {
        std::cerr << "Failed to listen on WebSocket server socket" << std::endl;
        close(server_socket_);
        return false;
    }
    
    return true;
}

bool WebSocketServer::start() {
    std::cout << "Starting WebSocket server on port " << port_ << std::endl;
    
    if (!initializeSocket()) {
        return false;
    }
    
    running_ = true;
    accept_thread_ = std::thread(&WebSocketServer::acceptConnections, this);
    
    std::cout << "🔌 WebSocket server listening on ws://0.0.0.0:" << port_ << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (running_) {
        running_ = false;
        
        // Close all client connections
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (auto& pair : connections_) {
                close(pair.first);
            }
            connections_.clear();
            user_sockets_.clear();
        }
        
        // Close server socket
        if (server_socket_ >= 0) {
            close(server_socket_);
            server_socket_ = -1;
        }
        
        // Wait for accept thread to finish
        if (accept_thread_.joinable()) {
            accept_thread_.join();
        }
        
        std::cout << "WebSocket server stopped" << std::endl;
    }
}

void WebSocketServer::acceptConnections() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Failed to accept WebSocket connection" << std::endl;
            }
            continue;
        }
        
        // Handle client in a new thread
        std::thread client_thread(&WebSocketServer::handleClient, this, client_socket);
        client_thread.detach();
    }
}

void WebSocketServer::handleClient(int client_socket) {
    std::string request;
    
    // Perform WebSocket handshake
    if (!performWebSocketHandshake(client_socket, request)) {
        std::cerr << "WebSocket handshake failed" << std::endl;
        close(client_socket);
        return;
    }
    
    // Authenticate the connection
    int user_id = authenticateConnection(request);
    if (user_id <= 0) {
        std::cerr << "WebSocket authentication failed" << std::endl;
        close(client_socket);
        return;
    }
    
    // Create connection object
    auto connection = std::make_shared<WebSocketConnection>(client_socket, user_id);
    
    // Add to connections map
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_[client_socket] = connection;
        user_sockets_[user_id].insert(client_socket);
    }
    
    std::cout << "WebSocket client connected: user_id=" << user_id 
              << ", socket=" << client_socket << std::endl;
    
    // Send online status notification
    WebSocketMessage online_msg("user:online", "{\"user_id\":" + std::to_string(user_id) + "}");
    broadcast(online_msg);
    
    // Read messages from client
    char buffer[4096];
    while (running_) {
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0) {
            break; // Connection closed or error
        }
        
        try {
            std::string frame(buffer, bytes_read);
            std::string decoded = decodeFrame(frame);
            
            if (!decoded.empty()) {
                WebSocketMessage message = parseMessage(decoded);
                
                // Find handler for this message type
                MessageHandler handler;
                {
                    std::lock_guard<std::mutex> lock(handlers_mutex_);
                    auto it = handlers_.find(message.type);
                    if (it != handlers_.end()) {
                        handler = it->second;
                    }
                }
                
                // Call handler if found
                if (handler) {
                    handler(user_id, message);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing WebSocket message: " << e.what() << std::endl;
        }
    }
    
    // Remove connection
    removeConnection(client_socket);
    close(client_socket);
    
    // Send offline status notification
    WebSocketMessage offline_msg("user:offline", "{\"user_id\":" + std::to_string(user_id) + "}");
    broadcast(offline_msg);
    
    std::cout << "WebSocket client disconnected: user_id=" << user_id << std::endl;
}

bool WebSocketServer::performWebSocketHandshake(int client_socket, std::string& request) {
    char buffer[4096];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return false;
    }
    
    buffer[bytes_read] = '\0';
    request = std::string(buffer);
    
    // Extract Sec-WebSocket-Key
    std::regex key_regex("Sec-WebSocket-Key: ([^\r\n]+)");
    std::smatch matches;
    if (!std::regex_search(request, matches, key_regex)) {
        return false;
    }
    
    std::string key = matches[1].str();
    
    // Extract Origin header for CORS
    std::string cors_origin = "*";
    std::regex origin_regex("Origin: ([^\r\n]+)");
    std::smatch origin_matches;
    if (std::regex_search(request, origin_matches, origin_regex)) {
        cors_origin = origin_matches[1].str();
    }
    
    // Compute accept key
    std::string accept_string = key + WEBSOCKET_GUID;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)accept_string.c_str(), accept_string.length(), hash);
    std::string accept_key = base64_encode(hash, SHA_DIGEST_LENGTH);
    
    // Send handshake response with CORS headers
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n";
    response << "Upgrade: websocket\r\n";
    response << "Connection: Upgrade\r\n";
    response << "Sec-WebSocket-Accept: " << accept_key << "\r\n";
    response << "Access-Control-Allow-Origin: " << cors_origin << "\r\n";
    
    // Only add credentials header if origin is not "*"
    if (cors_origin != "*") {
        response << "Access-Control-Allow-Credentials: true\r\n";
    }
    
    response << "\r\n";
    
    std::string response_str = response.str();
    ssize_t sent = send(client_socket, response_str.c_str(), response_str.length(), 0);
    
    return sent > 0;
}

int WebSocketServer::authenticateConnection(const std::string& request) {
    // Extract token from request URL or headers
    // Try to extract from URL first: ws://host:port/?token=xxx
    std::regex token_regex("GET /\\?token=([^ ]+)");
    std::smatch matches;
    
    std::string token;
    if (std::regex_search(request, matches, token_regex)) {
        token = matches[1].str();
    } else {
        // Try to extract from Authorization header
        std::regex auth_regex("Authorization: Bearer ([^\r\n]+)");
        if (std::regex_search(request, matches, auth_regex)) {
            token = matches[1].str();
        }
    }
    
    if (token.empty()) {
        return -1;
    }
    
    // Verify JWT token
    try {
        std::string jwt_secret = config::get_jwt_secret();
        auto payload = security::verify_jwt_token(token, jwt_secret);
        if (payload.has_value()) {
            return payload->user_id;
        }
    } catch (const std::exception& e) {
        std::cerr << "JWT verification failed: " << e.what() << std::endl;
    }
    
    return -1;
}

WebSocketMessage WebSocketServer::parseMessage(const std::string& raw_message) {
    // Simple JSON-like parsing: {"type":"xxx","payload":{...}}
    std::regex type_regex("\"type\"\\s*:\\s*\"([^\"]+)\"");
    std::regex payload_regex("\"payload\"\\s*:\\s*(\\{.*\\}|\"[^\"]*\")");
    
    std::smatch matches;
    std::string type, payload;
    
    if (std::regex_search(raw_message, matches, type_regex)) {
        type = matches[1].str();
    }
    
    if (std::regex_search(raw_message, matches, payload_regex)) {
        payload = matches[1].str();
    }
    
    return WebSocketMessage(type, payload);
}

std::string WebSocketServer::formatMessage(const WebSocketMessage& message) {
    std::ostringstream oss;
    oss << "{\"type\":\"" << message.type << "\",\"payload\":" << message.payload << "}";
    return oss.str();
}

std::string WebSocketServer::decodeFrame(const std::string& frame) {
    if (frame.length() < 2) {
        return "";
    }
    
    // unsigned char byte1 = frame[0]; // FIN bit + opcode (not used currently)
    unsigned char byte2 = frame[1];
    
    // Check if frame is masked (required for client-to-server)
    bool masked = (byte2 & 0x80) != 0;
    uint64_t payload_length = byte2 & 0x7F;
    
    size_t header_size = 2;
    
    // Extended payload length
    if (payload_length == 126) {
        if (frame.length() < 4) return "";
        payload_length = ((unsigned char)frame[2] << 8) | (unsigned char)frame[3];
        header_size = 4;
    } else if (payload_length == 127) {
        if (frame.length() < 10) return "";
        payload_length = 0;
        for (int i = 0; i < 8; i++) {
            payload_length = (payload_length << 8) | (unsigned char)frame[2 + i];
        }
        header_size = 10;
    }
    
    // Extract masking key if present
    unsigned char masking_key[4] = {0};
    if (masked) {
        if (frame.length() < header_size + 4) return "";
        for (int i = 0; i < 4; i++) {
            masking_key[i] = frame[header_size + i];
        }
        header_size += 4;
    }
    
    // Check if we have the full payload
    if (frame.length() < header_size + payload_length) {
        return "";
    }
    
    // Decode payload
    std::string decoded;
    decoded.reserve(payload_length);
    for (uint64_t i = 0; i < payload_length; i++) {
        unsigned char byte = frame[header_size + i];
        if (masked) {
            byte ^= masking_key[i % 4];
        }
        decoded += byte;
    }
    
    return decoded;
}

std::string WebSocketServer::encodeFrame(const std::string& message) {
    std::string frame;
    
    // First byte: FIN bit + opcode (1 for text)
    frame += (char)0x81;
    
    // Payload length
    size_t len = message.length();
    if (len <= 125) {
        frame += (char)len;
    } else if (len <= 65535) {
        frame += (char)126;
        frame += (char)((len >> 8) & 0xFF);
        frame += (char)(len & 0xFF);
    } else {
        frame += (char)127;
        for (int i = 7; i >= 0; i--) {
            frame += (char)((len >> (i * 8)) & 0xFF);
        }
    }
    
    // Payload (server-to-client frames are not masked)
    frame += message;
    
    return frame;
}

void WebSocketServer::registerHandler(const std::string& type, MessageHandler handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_[type] = handler;
}

bool WebSocketServer::sendToUser(int user_id, const WebSocketMessage& message) {
    std::string encoded = encodeFrame(formatMessage(message));
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = user_sockets_.find(user_id);
    if (it == user_sockets_.end()) {
        return false;
    }
    
    bool sent = false;
    for (int socket_fd : it->second) {
        auto conn_it = connections_.find(socket_fd);
        if (conn_it != connections_.end()) {
            if (conn_it->second->sendMessage(encoded)) {
                sent = true;
            }
        }
    }
    
    return sent;
}

void WebSocketServer::sendToUsers(const std::set<int>& user_ids, const WebSocketMessage& message) {
    for (int user_id : user_ids) {
        sendToUser(user_id, message);
    }
}

void WebSocketServer::broadcast(const WebSocketMessage& message) {
    std::string encoded = encodeFrame(formatMessage(message));
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (auto& pair : connections_) {
        pair.second->sendMessage(encoded);
    }
}

bool WebSocketServer::isUserOnline(int user_id) const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = user_sockets_.find(user_id);
    return it != user_sockets_.end() && !it->second.empty();
}

std::set<int> WebSocketServer::getOnlineUsers() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    std::set<int> online_users;
    for (const auto& pair : user_sockets_) {
        if (!pair.second.empty()) {
            online_users.insert(pair.first);
        }
    }
    return online_users;
}

void WebSocketServer::removeConnection(int socket_fd) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = connections_.find(socket_fd);
    if (it != connections_.end()) {
        int user_id = it->second->getUserId();
        
        // Remove from user_sockets_
        auto user_it = user_sockets_.find(user_id);
        if (user_it != user_sockets_.end()) {
            user_it->second.erase(socket_fd);
            if (user_it->second.empty()) {
                user_sockets_.erase(user_it);
            }
        }
        
        // Remove from connections_
        connections_.erase(it);
    }
}

} // namespace server
} // namespace sohbet
