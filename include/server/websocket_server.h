#pragma once

#include <string>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

namespace sohbet {
namespace server {

/**
 * WebSocket message structure
 */
struct WebSocketMessage {
    std::string type;
    std::string payload;
    
    WebSocketMessage(const std::string& t, const std::string& p)
        : type(t), payload(p) {}
};

/**
 * WebSocket connection representing a client
 */
class WebSocketConnection {
public:
    WebSocketConnection(int socket_fd, int user_id)
        : socket_fd_(socket_fd), user_id_(user_id), authenticated_(true) {}
    
    int getSocketFd() const { return socket_fd_; }
    int getUserId() const { return user_id_; }
    bool isAuthenticated() const { return authenticated_; }
    
    bool sendMessage(const std::string& message);
    
private:
    int socket_fd_;
    int user_id_;
    bool authenticated_;
};

/**
 * WebSocket Server for real-time communication
 */
class WebSocketServer {
public:
    using MessageHandler = std::function<void(int user_id, const WebSocketMessage& message)>;
    using DisconnectHandler = std::function<void(int user_id)>;

    /**
     * Constructor
     * @param port Port to listen on for WebSocket connections
     */
    WebSocketServer(int port = 8081);
    
    /**
     * Destructor
     */
    ~WebSocketServer();
    
    /**
     * Start the WebSocket server
     * @return true if successful
     */
    bool start();
    
    /**
     * Stop the WebSocket server
     */
    void stop();
    
    /**
     * Register a message handler for specific message types
     * @param type Message type to handle
     * @param handler Handler function
     */
    void registerHandler(const std::string& type, MessageHandler handler);

    /**
     * Register a disconnect handler called when a user disconnects
     * @param handler Handler function called with user_id
     */
    void registerDisconnectHandler(DisconnectHandler handler);

    /**
     * Send message to a specific user
     * @param user_id Target user ID
     * @param message Message to send
     * @return true if sent successfully
     */
    bool sendToUser(int user_id, const WebSocketMessage& message);
    
    /**
     * Send message to multiple users
     * @param user_ids Set of user IDs to send to
     * @param message Message to send
     */
    void sendToUsers(const std::set<int>& user_ids, const WebSocketMessage& message);
    
    /**
     * Broadcast message to all connected users
     * @param message Message to broadcast
     */
    void broadcast(const WebSocketMessage& message);
    
    /**
     * Check if a user is online
     * @param user_id User ID to check
     * @return true if user is connected
     */
    bool isUserOnline(int user_id) const;
    
    /**
     * Get all online users
     * @return Set of online user IDs
     */
    std::set<int> getOnlineUsers() const;

private:
    int port_;
    int server_socket_;
    std::atomic<bool> running_;
    std::thread accept_thread_;
    
    // Connection management
    mutable std::mutex connections_mutex_;
    std::map<int, std::shared_ptr<WebSocketConnection>> connections_; // socket_fd -> connection
    std::map<int, std::set<int>> user_sockets_; // user_id -> set of socket_fds
    
    // Message handlers
    mutable std::mutex handlers_mutex_;
    std::map<std::string, MessageHandler> handlers_;
    DisconnectHandler disconnect_handler_;

    // Server methods
    bool initializeSocket();
    void acceptConnections();
    void handleClient(int client_socket);
    bool performWebSocketHandshake(int client_socket, std::string& request);
    int authenticateConnection(const std::string& request);
    WebSocketMessage parseMessage(const std::string& raw_message);
    std::string formatMessage(const WebSocketMessage& message);
    
    // WebSocket frame handling
    std::string decodeFrame(const std::string& frame);
    std::string encodeFrame(const std::string& message);
    
    // Connection cleanup
    void removeConnection(int socket_fd);
};

} // namespace server
} // namespace sohbet
