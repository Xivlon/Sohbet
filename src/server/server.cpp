#include "server/server.h"
#include "models/user.h"
#include "models/media.h"
#include "models/group.h"
#include "models/organization.h"
#include "models/email_verification_token.h"
#include "repositories/email_verification_token_repository.h"
#include "services/email_service.h"
#include "security/jwt.h"
#include "config/env.h"
#include "utils/hash.h"
#include "utils/multipart_parser.h"
#include "utils/text_parser.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <chrono>

namespace sohbet {
namespace server {

// Helper function to escape JSON strings
static std::string escapeJsonString(const std::string& input) {
    std::ostringstream output;
    for (char c : input) {
        switch (c) {
            case '"':  output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                // Escape control characters
                if (c < 0x20) {
                    output << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    output << c;
                }
                break;
        }
    }
    return output.str();
}

AcademicSocialServer::AcademicSocialServer(int port, const std::string& connection_string)
    : port_(port), connection_string_(connection_string), running_(false), server_socket_(-1) {
}

bool AcademicSocialServer::initialize() {
    database_ = std::make_shared<db::Database>(connection_string_);
    if (!database_->isOpen()) {
        std::cerr << "Failed to open database with connection string" << std::endl;
        return false;
    }

    user_repository_ = std::make_shared<repositories::UserRepository>(database_);
    media_repository_ = std::make_shared<repositories::MediaRepository>(database_);
    friendship_repository_ = std::make_shared<repositories::FriendshipRepository>(database_);
    post_repository_ = std::make_shared<repositories::PostRepository>(database_);
    comment_repository_ = std::make_shared<repositories::CommentRepository>(database_);
    group_repository_ = std::make_shared<repositories::GroupRepository>(database_);
    organization_repository_ = std::make_shared<repositories::OrganizationRepository>(database_);
    role_repository_ = std::make_shared<repositories::RoleRepository>(database_);
    conversation_repository_ = std::make_shared<repositories::ConversationRepository>(database_);
    message_repository_ = std::make_shared<repositories::MessageRepository>(database_);
    voice_channel_repository_ = std::make_shared<repositories::VoiceChannelRepository>(database_);
    notification_repository_ = std::make_shared<repositories::NotificationRepository>(database_);
    user_presence_repository_ = std::make_shared<repositories::UserPresenceRepository>(database_);
    study_session_repository_ = std::make_shared<repositories::StudySessionRepository>(database_);
    hashtag_repository_ = std::make_shared<repositories::HashtagRepository>(database_);
    mention_repository_ = std::make_shared<repositories::MentionRepository>(database_);
    announcement_repository_ = std::make_shared<repositories::AnnouncementRepository>(database_);
    study_preferences_repository_ = std::make_shared<repositories::StudyPreferencesRepository>(database_);
    study_buddy_match_repository_ = std::make_shared<repositories::StudyBuddyMatchRepository>(database_);
    study_buddy_connection_repository_ = std::make_shared<repositories::StudyBuddyConnectionRepository>(database_);
    email_verification_token_repository_ = std::make_shared<repositories::EmailVerificationTokenRepository>(database_);
    storage_service_ = std::make_shared<services::StorageService>("uploads/");
    // TODO: Enable when CURL is available
    // email_service_ = std::make_shared<services::EmailService>();
    email_service_ = nullptr;
    std::cout << "Note: Email service not available in this build (requires libcurl-dev)" << std::endl;
    study_buddy_matching_service_ = std::make_shared<services::StudyBuddyMatchingService>(
        study_preferences_repository_,
        study_buddy_match_repository_,
        user_repository_
    );

    // Initialize voice service
    VoiceConfig voice_config;
    voice_config.load_from_env();
    voice_config.enabled = true;
    voice_service_ = std::make_shared<VoiceServiceStub>(voice_config);
    
    // Initialize WebSocket server with configurable port
    int ws_port = config::get_websocket_port();
    websocket_server_ = std::make_shared<WebSocketServer>(ws_port);
    setupWebSocketHandlers();

    if (!user_repository_->migrate()) {
        std::cerr << "Failed to run database migrations" << std::endl;
        return false;
    }
    
    // Run social features migration if needed
    // Use a relative path from the current working directory
    const std::string migration_path = "migrations/001_social_features.sql";
    std::ifstream migration_file(migration_path);
    if (migration_file.is_open()) {
        std::stringstream buffer;
        buffer << migration_file.rdbuf();
        std::string migration_sql = buffer.str();
        migration_file.close();

        if (!database_->execute(migration_sql)) {
            std::cerr << "Warning: Social features migration failed (may already be applied)" << std::endl;
        } else {
            std::cout << "Social features migration applied successfully" << std::endl;
        }
    }

    // Run enhanced features migration if needed
    const std::string enhanced_migration_path = "migrations/004_enhanced_features.sql";
    std::ifstream enhanced_migration_file(enhanced_migration_path);
    if (enhanced_migration_file.is_open()) {
        std::stringstream buffer;
        buffer << enhanced_migration_file.rdbuf();
        std::string migration_sql = buffer.str();
        enhanced_migration_file.close();

        if (!database_->execute(migration_sql)) {
            std::cerr << "Warning: Enhanced features migration failed (may already be applied)" << std::endl;
        } else {
            std::cout << "Enhanced features migration applied successfully" << std::endl;
        }
    }

    // Run study buddy matching migration if needed
    const std::string study_buddy_migration_path = "migrations/005_study_buddy_matching.sql";
    std::ifstream study_buddy_migration_file(study_buddy_migration_path);
    if (study_buddy_migration_file.is_open()) {
        std::stringstream buffer;
        buffer << study_buddy_migration_file.rdbuf();
        std::string migration_sql = buffer.str();
        study_buddy_migration_file.close();

        if (!database_->execute(migration_sql)) {
            std::cerr << "Warning: Study buddy matching migration failed (may already be applied)" << std::endl;
        } else {
            std::cout << "Study buddy matching migration applied successfully" << std::endl;
        }
    }

    // Ensure demo users exist for demo/testing purposes
    ensureDemoUserExists();
    ensureSecondDemoUserExists();

    std::cout << "Server initialized successfully" << std::endl;
    return true;
}

bool AcademicSocialServer::start() {
    std::cout << "Academic Social Server starting on port " << port_ << std::endl;
    std::cout << "Database: PostgreSQL (connection configured)" << std::endl;
    std::cout << "Version: 0.3.0-academic" << std::endl;
    
    if (!initializeSocket()) {
        std::cerr << "Failed to initialize server socket" << std::endl;
        return false;
    }
    
    // Start WebSocket server
    if (!websocket_server_->start()) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        return false;
    }

    // Start voice channel cleanup task
    cleanup_running_ = true;
    voice_cleanup_thread_ = std::thread(&AcademicSocialServer::runVoiceChannelCleanup, this);

    running_ = true;
    std::cout << "🌐 HTTP Server listening on http://0.0.0.0:" << port_ << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /api/status" << std::endl;
    std::cout << "  GET  /api/users (list all users)" << std::endl;
    std::cout << "  GET  /api/users/demo" << std::endl;
    std::cout << "  POST /api/users (registration)" << std::endl;
    std::cout << "  POST /api/login" << std::endl;
    std::cout << "  PUT  /api/users/:id (update profile)" << std::endl;
    std::cout << "  POST /api/media/upload (file upload)" << std::endl;
    std::cout << "  GET  /api/media/file/:key (retrieve file)" << std::endl;
    std::cout << "  GET  /api/users/:id/media (user's media)" << std::endl;
    std::cout << "Server ready to handle requests" << std::endl;
    
    // Accept connections
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Error accepting connection" << std::endl;
            }
            continue;
        }
        
        // Handle client in a separate thread for concurrent connections
        std::thread client_thread(&AcademicSocialServer::handleClient, this, client_socket);
        client_thread.detach();
    }
    
    return true;
}

void AcademicSocialServer::stop() {
    running_ = false;

    // Stop voice channel cleanup task
    cleanup_running_ = false;
    if (voice_cleanup_thread_.joinable()) {
        voice_cleanup_thread_.join();
    }

    // Stop WebSocket server
    if (websocket_server_) {
        websocket_server_->stop();
    }

    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    std::cout << "Server stopped" << std::endl;
}

bool AcademicSocialServer::initializeSocket() {
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options: " << strerror(errno) << std::endl;
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
        std::cerr << "Error binding socket to port " << port_ << ": " << strerror(errno) << std::endl;
        close(server_socket_);
        return false;
    }
    
    // Listen for connections
    if (listen(server_socket_, 10) < 0) {
        std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
        close(server_socket_);
        return false;
    }
    
    return true;
}

void AcademicSocialServer::handleClient(int client_socket) {
    std::cerr << "DEBUG: handleClient started" << std::endl;
    const int INITIAL_BUFFER_SIZE = 8192;
    const size_t MAX_REQUEST_SIZE = 10 * 1024 * 1024; // 10MB max request size
    std::vector<char> buffer(INITIAL_BUFFER_SIZE);
    std::string raw_request;
    
    // Set socket timeout to prevent indefinite blocking
    struct timeval timeout;
    timeout.tv_sec = 30;  // 30 second timeout
    timeout.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // Read request data
    ssize_t total_bytes = 0;
    ssize_t bytes_read;
    bool headers_found = false;
    size_t headers_end = 0;
    
    while ((bytes_read = recv(client_socket, buffer.data() + total_bytes, 
                               buffer.size() - total_bytes, 0)) > 0) {
        total_bytes += bytes_read;
        
        // Only search for headers if we haven't found them yet
        if (!headers_found) {
            // Use string_view-like approach to avoid creating new strings
            const char* data_ptr = buffer.data();
            for (size_t i = 0; i + 3 < static_cast<size_t>(total_bytes); ++i) {
                if (data_ptr[i] == '\r' && data_ptr[i+1] == '\n' && 
                    data_ptr[i+2] == '\r' && data_ptr[i+3] == '\n') {
                    headers_end = i;
                    headers_found = true;
                    break;
                }
            }
        }
        
        if (headers_found) {
            // Parse headers to get Content-Length
            std::string headers_section(buffer.data(), headers_end);
            std::istringstream header_stream(headers_section);
            std::string line;
            size_t content_length = 0;
            bool has_content_length = false;
            
            while (std::getline(header_stream, line)) {
                if (line.find("Content-Length:") == 0) {
                    std::string length_str = line.substr(15);
                    // Trim whitespace
                    length_str.erase(0, length_str.find_first_not_of(" \t\r\n"));
                    length_str.erase(length_str.find_last_not_of(" \t\r\n") + 1);
                    
                    try {
                        content_length = std::stoull(length_str);
                        has_content_length = true;
                    } catch (const std::exception&) {
                        // Invalid Content-Length, close connection
                        close(client_socket);
                        return;
                    }
                    break;
                }
            }
            
            if (has_content_length) {
                size_t expected_total = headers_end + 4 + content_length; // +4 for \r\n\r\n
                
                // Enforce maximum request size
                if (expected_total > MAX_REQUEST_SIZE) {
                    // Send 413 Payload Too Large
                    std::string error_response = 
                        "HTTP/1.1 413 Payload Too Large\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: 38\r\n"
                        "Connection: close\r\n\r\n"
                        "{\"error\":\"Request too large (max 10MB)\"}";
                    send(client_socket, error_response.c_str(), error_response.length(), 0);
                    close(client_socket);
                    return;
                }
                
                // Resize buffer if needed
                if (expected_total > buffer.size()) {
                    buffer.resize(expected_total);
                }
                
           // Keep reading until we have all the data
          while (total_bytes < static_cast<ssize_t>(expected_total)) {
            bytes_read = recv(client_socket, buffer.data() + total_bytes,
                     expected_total - total_bytes, 0);
        if (bytes_read < 0) {
            std::cerr << "recv() error: " << strerror(errno) << std::endl;
        break;
        }
        if (bytes_read == 0) break; // EOF
            total_bytes += bytes_read;
            }
        }
            
            break; // We have the full request
        }
        
        // Expand buffer if needed, but enforce max size
        if (total_bytes >= static_cast<ssize_t>(buffer.size())) {
            if (buffer.size() * 2 > MAX_REQUEST_SIZE) {
                close(client_socket);
                return;
            }
            buffer.resize(buffer.size() * 2);
        }
    }
    
    if (total_bytes <= 0) {
        close(client_socket);
        return;
    }
    
    raw_request = std::string(buffer.data(), total_bytes);
    
    // Parse HTTP request
    HttpRequest request = parseHttpRequest(raw_request);
    
    // Handle request
    HttpResponse response = handleRequest(request);

    std::cerr << "DEBUG: Response created - Status: " << response.status_code
              << ", Content-Type: " << response.content_type
              << ", Body length: " << response.body.length() << std::endl;
    if (response.body.length() > 0 && response.body.length() <= 200) {
        std::cerr << "DEBUG: Response body: " << response.body << std::endl;
    }

    // Format and send response
    std::string http_response = formatHttpResponse(response, request);
    std::cerr << "DEBUG: Formatted HTTP response length: " << http_response.length() << " bytes" << std::endl;

    ssize_t bytes_sent = send(client_socket, http_response.c_str(), http_response.length(), 0);
    if (bytes_sent < 0) {
        std::cerr << "ERROR: Failed to send response: " << strerror(errno) << std::endl;
    } else {
        std::cerr << "DEBUG: Successfully sent " << bytes_sent << " bytes to client" << std::endl;
    }

    close(client_socket);
}

HttpRequest AcademicSocialServer::parseHttpRequest(const std::string& raw_request) {
    std::cerr << "DEBUG: parseHttpRequest called with " << raw_request.length() << " bytes" << std::endl;
    std::cerr << "DEBUG: Raw request (first 500 chars):\n" << raw_request.substr(0, 500) << std::endl;
    
    size_t headers_end = raw_request.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        std::cerr << "  WARNING: No \\r\\n\\r\\n found!" << std::endl;
    } else {
        std::cerr << "  Headers end at position: " << headers_end << std::endl;
    }
    
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line (GET /path HTTP/1.1)
    std::getline(stream, line);
    std::istringstream request_line(line);
    std::string method, path, version;
    request_line >> method >> path >> version;
    
    std::cerr << "DEBUG: Request line - Method: " << method << ", Path: " << path << std::endl;
    
    HttpRequest request(method, path, "");
    
    // Parse headers
    bool headers_done = false;
    int header_count = 0;
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) {
            headers_done = true;
            break;
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // DEBUG: Log every header received
        std::cerr << "DEBUG: Header #" << (++header_count) << ": " << line.substr(0, 150) << std::endl;
        
        // Parse header (Name: Value)
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);
            
            // Trim whitespace from value
            size_t start = header_value.find_first_not_of(" \t");
            if (start != std::string::npos) {
                header_value = header_value.substr(start);
            }
            
            std::cerr << "DEBUG: Storing header '" << header_name << "' = '" << header_value.substr(0, 100) << "'" << std::endl;
            request.headers[header_name] = header_value;
        }
    }
    
    std::cerr << "DEBUG: Total headers parsed: " << header_count << std::endl;
    
    // Read body if present
    if (headers_done) {
        std::string remaining;
        while (std::getline(stream, line)) {
            request.body += line + "\n";
        }
    }
    
    return request;
}

// Helper function to validate and sanitize Origin header
static bool isValidOrigin(const std::string& origin) {
    // Validate that the origin matches a proper URL format
    // This prevents header injection attacks while still allowing all valid origins
    // Use static regex to compile once and reuse for performance
    static const std::regex origin_regex(R"(^https?://[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*(:[0-9]{1,5})?$)");
    return std::regex_match(origin, origin_regex);
}

std::string AcademicSocialServer::formatHttpResponse(const HttpResponse& response, const HttpRequest& request) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code << " ";
    
    // Status text
    switch (response.status_code) {
        case 200: oss << "OK"; break;
        case 201: oss << "Created"; break;
        case 204: oss << "No Content"; break;
        case 400: oss << "Bad Request"; break;
        case 401: oss << "Unauthorized"; break;
        case 404: oss << "Not Found"; break;
        case 500: oss << "Internal Server Error"; break;
        default: oss << "Unknown"; break;
    }
    
    oss << "\r\n";
    oss << "Content-Type: " << response.content_type << "\r\n";
    oss << "Content-Length: " << response.body.length() << "\r\n";
    
    // CORS headers - allow ALL origins by echoing the Origin header
    // Validate the origin to prevent header injection attacks
    std::string cors_origin = "*";
    auto origin_it = request.headers.find("Origin");
    if (origin_it != request.headers.end() && isValidOrigin(origin_it->second)) {
        cors_origin = origin_it->second;
    }
    
    oss << "Access-Control-Allow-Origin: " << cors_origin << "\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    
    // Only add credentials header if origin is not "*"
    if (cors_origin != "*") {
        oss << "Access-Control-Allow-Credentials: true\r\n";
    }
    
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
}

// -------------------- Request Handlers --------------------
// Add this function right before handleGetPosts() (before line 420)
HttpResponse AcademicSocialServer::handleRequest(const HttpRequest& request) {
    // Extract base path (without query string)
    std::string base_path = request.path;
    size_t query_pos = base_path.find('?');
    if (query_pos != std::string::npos) {
        base_path = base_path.substr(0, query_pos);
    }
    
    // Handle CORS preflight requests FIRST (before logging/processing body)
    if (request.method == "OPTIONS") {
        std::cerr << "DEBUG: Handling OPTIONS preflight for " << base_path << std::endl;
        return HttpResponse(200, "text/plain", "");
    }
    
    // Now log debug info for actual requests
    std::cerr << "DEBUG: handleRequest called" << std::endl;
    std::cerr << "  Method: " << request.method << std::endl;
    std::cerr << "  Path: " << request.path << std::endl;
    std::cerr << "  Base Path: " << base_path << std::endl;
    std::cerr << "  Headers count: " << request.headers.size() << std::endl;
    std::cerr << "  Body length: " << request.body.length() << std::endl;
    if (request.body.length() > 0) {
        std::cerr << "  Body content (first 200 chars): " << request.body.substr(0, 200) << std::endl;
    }

    // Status endpoint - no authentication required for health checks
    if (request.method == "GET" && base_path == "/api/status") {
        return handleStatus(request);
    }

    int author_id = getUserIdFromAuth(request);

    if (request.method == "GET" && base_path == "/api/users") {
        return handleGetUsers(request);
    } else if (request.method == "GET" && base_path == "/api/users/demo") {
        return handleUsersDemo(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/friends") != std::string::npos) {
        return handleGetFriends(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/posts") != std::string::npos) {
        return handleGetUserPosts(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/media") != std::string::npos) {
        return handleGetUserMedia(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/friends") == std::string::npos && base_path.find("/posts") == std::string::npos && base_path.find("/media") == std::string::npos) {
        return handleGetUserById(request);
    } else if (request.method == "POST" && base_path == "/api/users") {
        return handleCreateUser(request);
    } else if (request.method == "PUT" && base_path.find("/api/users/") == 0) {
        return handleUpdateUser(request);
    } else if (request.method == "POST" && base_path == "/api/login") {
        return handleLogin(request);
    } else if (request.method == "POST" && base_path == "/api/verify-email") {
        return handleVerifyEmail(request);
    } else if (request.method == "POST" && base_path == "/api/media/upload") {
        return handleUploadMedia(request);
    } else if (request.method == "GET" && base_path.find("/api/media/file/") == 0) {
        return handleGetMediaFile(request);
    }
    // Friendship routes
    else if (request.method == "POST" && base_path == "/api/friendships") {
        return handleCreateFriendship(request);
    } else if (request.method == "GET" && base_path == "/api/friendships") {
        return handleGetFriendships(request);
    } else if (request.method == "PUT" && base_path.find("/api/friendships/") == 0 && base_path.find("/accept") != std::string::npos) {
        return handleAcceptFriendship(request);
    } else if (request.method == "PUT" && base_path.find("/api/friendships/") == 0 && base_path.find("/reject") != std::string::npos) {
        return handleRejectFriendship(request);
    } else if (request.method == "DELETE" && base_path.find("/api/friendships/") == 0) {
        return handleDeleteFriendship(request);
    }
    // Post routes
    else if (request.method == "POST" && base_path == "/api/posts") {
        return handleCreatePost(request);
    } else if (request.method == "GET" && base_path == "/api/posts") {
        return handleGetPosts(request);
    } else if (request.method == "PUT" && base_path.find("/api/posts/") == 0 && base_path.find("/react") == std::string::npos) {
        return handleUpdatePost(request);
    } else if (request.method == "DELETE" && base_path.find("/api/posts/") == 0 && base_path.find("/react") == std::string::npos) {
        return handleDeletePost(request);
    } else if (request.method == "POST" && base_path.find("/api/posts/") == 0 && base_path.find("/react") != std::string::npos) {
        return handleAddReaction(request);
    } else if (request.method == "DELETE" && base_path.find("/api/posts/") == 0 && base_path.find("/react") != std::string::npos) {
        return handleRemoveReaction(request);
    }
    // Comment routes
    else if (request.method == "POST" && base_path.find("/api/posts/") == 0 && base_path.find("/comments") != std::string::npos && base_path.find("/api/comments/") == std::string::npos) {
        return handleCreateComment(request);
    } else if (request.method == "GET" && base_path.find("/api/posts/") == 0 && base_path.find("/comments") != std::string::npos) {
        return handleGetComments(request);
    } else if (request.method == "POST" && base_path.find("/api/comments/") == 0 && base_path.find("/reply") != std::string::npos) {
        return handleReplyToComment(request);
    } else if (request.method == "PUT" && base_path.find("/api/comments/") == 0) {
        return handleUpdateComment(request);
    } else if (request.method == "DELETE" && base_path.find("/api/comments/") == 0) {
        return handleDeleteComment(request);
    }
    // Group routes
    else if (request.method == "POST" && base_path == "/api/groups") {
        return handleCreateGroup(request);
    } else if (request.method == "GET" && base_path == "/api/groups") {
        return handleGetGroups(request);
    } else if (request.method == "GET" && base_path.find("/api/groups/") == 0 && base_path.find("/members") == std::string::npos) {
        return handleGetGroup(request);
    } else if (request.method == "PUT" && base_path.find("/api/groups/") == 0 && base_path.find("/members") == std::string::npos) {
        return handleUpdateGroup(request);
    } else if (request.method == "DELETE" && base_path.find("/api/groups/") == 0 && base_path.find("/members") == std::string::npos) {
        return handleDeleteGroup(request);
    } else if (request.method == "POST" && base_path.find("/api/groups/") == 0 && base_path.find("/members") != std::string::npos) {
        return handleAddGroupMember(request);
    } else if (request.method == "DELETE" && base_path.find("/api/groups/") == 0 && base_path.find("/members/") != std::string::npos) {
        return handleRemoveGroupMember(request);
    } else if (request.method == "PUT" && base_path.find("/api/groups/") == 0 && base_path.find("/members/") != std::string::npos && base_path.find("/role") != std::string::npos) {
        return handleUpdateGroupMemberRole(request);
    }
    // Hashtag routes
    else if (request.method == "GET" && base_path == "/api/hashtags/trending") {
        return handleGetTrendingHashtags(request);
    } else if (request.method == "GET" && base_path == "/api/hashtags/search") {
        return handleSearchHashtags(request);
    } else if (request.method == "GET" && base_path.find("/api/hashtags/") == 0 && base_path.find("/posts") != std::string::npos) {
        return handleGetPostsByHashtag(request);
    }
    // Announcement routes
    else if (request.method == "POST" && base_path.find("/api/groups/") == 0 && base_path.find("/announcements") != std::string::npos && base_path.find("/api/groups/") == 0) {
        return handleCreateAnnouncement(request);
    } else if (request.method == "GET" && base_path.find("/api/groups/") == 0 && base_path.find("/announcements") != std::string::npos) {
        return handleGetAnnouncements(request);
    } else if (request.method == "GET" && base_path.find("/api/announcements/") == 0) {
        return handleGetAnnouncement(request);
    } else if (request.method == "PUT" && base_path.find("/api/announcements/") == 0 && base_path.find("/pin") == std::string::npos) {
        return handleUpdateAnnouncement(request);
    } else if (request.method == "DELETE" && base_path.find("/api/announcements/") == 0) {
        return handleDeleteAnnouncement(request);
    } else if (request.method == "PUT" && base_path.find("/api/announcements/") == 0 && base_path.find("/pin") != std::string::npos) {
        return handlePinAnnouncement(request);
    } else if (request.method == "PUT" && base_path.find("/api/announcements/") == 0 && base_path.find("/unpin") != std::string::npos) {
        return handleUnpinAnnouncement(request);
    }
    // Study Buddy routes
    else if (request.method == "GET" && base_path == "/api/study-buddies/preferences") {
        return handleGetStudyPreferences(request);
    } else if (request.method == "POST" && base_path == "/api/study-buddies/preferences") {
        return handleSetStudyPreferences(request);
    } else if (request.method == "GET" && base_path == "/api/study-buddies/matches") {
        return handleGetStudyBuddyMatches(request);
    } else if (request.method == "POST" && base_path == "/api/study-buddies/matches/refresh") {
        return handleRefreshStudyBuddyMatches(request);
    } else if (request.method == "PUT" && base_path.find("/api/study-buddies/matches/") == 0 && base_path.find("/accept") != std::string::npos) {
        return handleAcceptStudyBuddyMatch(request);
    } else if (request.method == "PUT" && base_path.find("/api/study-buddies/matches/") == 0 && base_path.find("/decline") != std::string::npos) {
        return handleDeclineStudyBuddyMatch(request);
    } else if (request.method == "GET" && base_path == "/api/study-buddies/connections") {
        return handleGetStudyBuddyConnections(request);
    }
    // Mention routes
    else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/mentions") != std::string::npos) {
        return handleGetUserMentions(request);
    }
    // Organization routes
    else if (request.method == "POST" && base_path == "/api/organizations") {
        return handleCreateOrganization(request);
    } else if (request.method == "GET" && base_path == "/api/organizations") {
        return handleGetOrganizations(request);
    } else if (request.method == "GET" && base_path.find("/api/organizations/") == 0 && base_path.find("/accounts") == std::string::npos) {
        return handleGetOrganization(request);
    } else if (request.method == "PUT" && base_path.find("/api/organizations/") == 0 && base_path.find("/accounts") == std::string::npos) {
        return handleUpdateOrganization(request);
    } else if (request.method == "DELETE" && base_path.find("/api/organizations/") == 0 && base_path.find("/accounts") == std::string::npos) {
        return handleDeleteOrganization(request);
    } else if (request.method == "POST" && base_path.find("/api/organizations/") == 0 && base_path.find("/accounts") != std::string::npos) {
        return handleAddOrganizationAccount(request);
    } else if (request.method == "DELETE" && base_path.find("/api/organizations/") == 0 && base_path.find("/accounts/") != std::string::npos) {
        return handleRemoveOrganizationAccount(request);
    }
    // Chat/Messaging routes
    else if (request.method == "GET" && base_path == "/api/conversations") {
        return handleGetConversations(request);
    } else if (request.method == "POST" && base_path == "/api/conversations") {
        return handleGetOrCreateConversation(request);
    } else if (request.method == "GET" && base_path.find("/api/conversations/") == 0 && base_path.find("/messages") != std::string::npos) {
        return handleGetMessages(request);
    } else if (request.method == "POST" && base_path.find("/api/conversations/") == 0 && base_path.find("/messages") != std::string::npos) {
        return handleSendMessage(request);
    } else if (request.method == "PUT" && base_path.find("/api/messages/") == 0 && base_path.find("/read") != std::string::npos) {
        return handleMarkMessageRead(request);
    }
    // Voice/Murmur routes
    else if (request.method == "POST" && base_path == "/api/voice/channels") {
        return handleCreateVoiceChannel(request);
    } else if (request.method == "GET" && base_path == "/api/voice/channels") {
        return handleGetVoiceChannels(request);
    } else if (request.method == "GET" && base_path.find("/api/voice/channels/") == 0 && base_path.find("/join") == std::string::npos && base_path.find("/leave") == std::string::npos) {
        return handleGetVoiceChannel(request);
    } else if (request.method == "POST" && base_path.find("/api/voice/channels/") == 0 && base_path.find("/join") != std::string::npos) {
        return handleJoinVoiceChannel(request);
    } else if (request.method == "DELETE" && base_path.find("/api/voice/channels/") == 0 && base_path.find("/leave") != std::string::npos) {
        return handleLeaveVoiceChannel(request);
    } else if (request.method == "DELETE" && base_path.find("/api/voice/channels/") == 0 && base_path.find("/leave") == std::string::npos) {
        return handleDeleteVoiceChannel(request);
    } else {
        return handleNotFound(request);
    }
}


HttpResponse AcademicSocialServer::handleStatus(const HttpRequest& request) {
    (void)request;
    std::string response = R"({"status":"ok","version":"0.3.0-academic","features":["user_registration","sqlite_persistence","bcrypt_hashing","websocket_chat","voice_channels","groups","organizations","real_time_messaging"]})";
    return createJsonResponse(200, response);
}

HttpResponse AcademicSocialServer::handleGetUsers(const HttpRequest& request) {
    int limit = 50;
    int offset = 0;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        int parsed_limit = std::stoi(match[1].str());
        if (parsed_limit > 0 && parsed_limit <= 100) {
            limit = parsed_limit;
        }
    }
    
    if (std::regex_search(request.path, match, offset_regex)) {
        int parsed_offset = std::stoi(match[1].str());
        if (parsed_offset >= 0) {
            offset = parsed_offset;
        }
    }
    
    std::vector<User> users = user_repository_->findAll(limit, offset);
    int total = user_repository_->countAll();
    
    std::ostringstream oss;
    oss << "{\"users\":[";
    
    for (size_t i = 0; i < users.size(); ++i) {
        oss << users[i].toJson();
        if (i < users.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "],\"total\":" << total;
    oss << ",\"limit\":" << limit;
    oss << ",\"offset\":" << offset;
    oss << ",\"count\":" << users.size() << "}";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleUsersDemo(const HttpRequest& request) {
    (void)request;
    User demo("demo_student", "demo@example.edu");
    demo.setId(999);
    demo.setUniversity("Demo University");
    demo.setDepartment("Computer Science");
    demo.setEnrollmentYear(2023);
    demo.setPrimaryLanguage("Turkish");
    demo.setPosition("Proffessor");
    return createJsonResponse(200, demo.toJson());
}

HttpResponse AcademicSocialServer::handleGetUserById(const HttpRequest& request) {
    try {
        // Extract user ID from path: /api/users/:id
        std::regex id_regex("/api/users/(\\d+)");
        std::smatch match;
        if (!std::regex_search(request.path, match, id_regex)) {
            return createErrorResponse(400, "Invalid user ID");
        }
        
        int user_id = std::stoi(match[1].str());
        
        auto user_opt = user_repository_->findById(user_id);
        if (!user_opt.has_value()) {
            return createErrorResponse(404, "User not found");
        }
        
        return createJsonResponse(200, user_opt.value().toJson());
    } catch (const std::exception& e) {
        std::cerr << "Error getting user by ID: " << e.what() << std::endl;
        return createErrorResponse(500, "Internal server error");
    } catch (...) {
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleCreateUser(const HttpRequest& request) {
    try {
        std::string username = extractJsonField(request.body, "username");
        std::string email = extractJsonField(request.body, "email");
        std::string password = extractJsonField(request.body, "password");

        std::string error;
        if (!validateUserRegistration(username, email, password, error)) {
            return createErrorResponse(400, error);
        }

        if (user_repository_->usernameExists(username)) return createErrorResponse(409, "Username already exists");
        if (user_repository_->emailExists(email)) return createErrorResponse(409, "Email already exists");

        User user;
        user.setUsername(username);
        user.setEmail(email);
        user.setPasswordHash(utils::hash_password(password));

        std::string university = extractJsonField(request.body, "university");
        if (!university.empty()) user.setUniversity(university);

        std::string department = extractJsonField(request.body, "department");
        if (!department.empty()) user.setDepartment(department);

        std::string year_str = extractJsonField(request.body, "enrollment_year");
        if (!year_str.empty()) user.setEnrollmentYear(std::stoi(year_str));

        std::string primary_lang = extractJsonField(request.body, "primary_language");
        if (!primary_lang.empty()) user.setPrimaryLanguage(primary_lang);

        std::regex lang_regex("\"additional_languages\"\\s*:\\s*\\[(.*?)\\]");
        std::smatch match;
        if (std::regex_search(request.body, match, lang_regex)) {
            std::string langs_str = match[1].str();
            std::vector<std::string> langs;
            std::regex item_regex("\"([^\"]*)\"");
            for (auto iter = std::sregex_iterator(langs_str.begin(), langs_str.end(), item_regex);
                 iter != std::sregex_iterator(); ++iter) {
                langs.push_back((*iter)[1].str());
            }
            user.setAdditionalLanguages(langs);
        }

        auto created_user = user_repository_->create(user, password);
        if (!created_user.has_value()) {
            return createErrorResponse(500, "Failed to create user");
        }

        // Create email verification token and send verification email
        int user_id = created_user.value().getId().value();
        auto token_opt = email_verification_token_repository_->createToken(user_id);
        if (token_opt.has_value()) {
            std::cout << "Email service not available in this build. Verification token created." << std::endl;
            std::cout << "Verification token for " << email << ": " << token_opt.value().getToken() << std::endl;
            std::cout << "In production, this token would be emailed to the user." << std::endl;
            // TODO: Uncomment when CURL is available
            // std::string user_name = created_user.value().getName().value_or(username);
            // if (email_service_) {
            //     bool email_sent = email_service_->sendVerificationEmail(
            //         email,
            //         user_name,
            //         token_opt.value().getToken()
            //     );
            //     if (email_sent) {
            //         std::cout << "Verification email sent to " << email << std::endl;
            //     } else {
            //         std::cerr << "Warning: Failed to send verification email to " << email << std::endl;
            //     }
            // }
        } else {
            std::cerr << "Warning: Failed to create verification token for user " << user_id << std::endl;
        }

        return createJsonResponse(201, created_user.value().toJson());
    } catch (...) {
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleLogin(const HttpRequest& request) {
    try {
        std::string username = extractJsonField(request.body, "username");
        std::string password = extractJsonField(request.body, "password");

        auto user_opt = user_repository_->findByUsername(username);
        if (!user_opt.has_value()) return createErrorResponse(401, "Invalid username or password");

        User user = user_opt.value();
        if (!utils::verify_password(password, user.getPasswordHash())) {
            return createErrorResponse(401, "Invalid username or password");
        }

        std::string user_role = user.getRole().value_or("Student");
        std::string jwt_secret = config::get_jwt_secret();
        int expiry_hours = config::get_jwt_expiry_hours();
        std::string token = security::generate_jwt_token(username, user.getId().value(), user_role, jwt_secret, expiry_hours);

        std::ostringstream oss;
        oss << "{ \"token\":\"" << token << "\", \"user\":" << user.toJson() << " }";
        return createJsonResponse(200, oss.str());
    } catch (const std::exception& e) {
        std::cerr << "Login error: " << e.what() << std::endl;
        return createErrorResponse(500, "Internal server error");
    } catch (...) {
        std::cerr << "Login error: Unknown exception" << std::endl;
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleVerifyEmail(const HttpRequest& request) {
    try {
        std::string token = extractJsonField(request.body, "token");

        if (token.empty()) {
            return createErrorResponse(400, "Token is required");
        }

        // Verify the token using the repository
        bool verified = email_verification_token_repository_->verifyToken(token);

        if (verified) {
            return createJsonResponse(200, "{\"message\":\"Email verified successfully\",\"verified\":true}");
        } else {
            // Token might be invalid, expired, or already used
            auto token_opt = email_verification_token_repository_->findByToken(token);
            if (!token_opt.has_value()) {
                return createErrorResponse(400, "Invalid verification token");
            }

            if (token_opt->isExpired()) {
                return createErrorResponse(400, "Verification token has expired");
            }

            if (token_opt->isVerified()) {
                return createErrorResponse(400, "Email has already been verified");
            }

            return createErrorResponse(400, "Failed to verify email");
        }
    } catch (const std::exception& e) {
        std::cerr << "Email verification error: " << e.what() << std::endl;
        return createErrorResponse(500, "Internal server error");
    } catch (...) {
        std::cerr << "Email verification error: Unknown exception" << std::endl;
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleUpdateUser(const HttpRequest& request) {
    try {
        std::regex id_regex("/api/users/(\\d+)");
        std::smatch match;
        if (!std::regex_search(request.path, match, id_regex)) {
            return createErrorResponse(400, "Invalid user ID");
        }
        
        int user_id = std::stoi(match[1].str());
        
        auto user_opt = user_repository_->findById(user_id);
        if (!user_opt.has_value()) {
            return createErrorResponse(404, "User not found");
        }
        
        User user = user_opt.value();
        
        std::string name = extractJsonField(request.body, "name");
        if (!name.empty()) user.setName(name);
        
        std::string position = extractJsonField(request.body, "position");
        if (!position.empty()) user.setPosition(position);
        
        std::string phone_number = extractJsonField(request.body, "phone_number");
        if (!phone_number.empty()) user.setPhoneNumber(phone_number);
        
        std::string university = extractJsonField(request.body, "university");
        if (!university.empty()) user.setUniversity(university);
        
        std::string department = extractJsonField(request.body, "department");
        if (!department.empty()) user.setDepartment(department);
        
        std::string enrollment_year_str = extractJsonField(request.body, "enrollment_year");
        if (!enrollment_year_str.empty()) {
            user.setEnrollmentYear(std::stoi(enrollment_year_str));
        }
        
        std::string primary_language = extractJsonField(request.body, "primary_language");
        if (!primary_language.empty()) user.setPrimaryLanguage(primary_language);
        
        if (!user_repository_->update(user)) {
            return createErrorResponse(500, "Failed to update user");
        }
        
        return createJsonResponse(200, user.toJson());
    } catch (...) {
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleNotFound(const HttpRequest& request) {
    (void)request;
    return createErrorResponse(404, "Endpoint not found");
}

// -------------------- Utilities --------------------

HttpResponse AcademicSocialServer::createJsonResponse(int status, const std::string& json) {
    return HttpResponse(status, "application/json", json);
}

HttpResponse AcademicSocialServer::createErrorResponse(int status, const std::string& message) {
    return createJsonResponse(status, "{\"error\":\"" + message + "\"}");
}

std::string AcademicSocialServer::extractJsonField(const std::string& json, const std::string& field) {
    std::cerr << "DEBUG: extractJsonField - field='" << field << "' json_len=" << json.length() << std::endl;
    std::cerr << "DEBUG: json content: '" << json.substr(0, std::min((size_t)300, json.length())) << "'" << std::endl;
    
    // Try to find the field with double quotes
    std::string search_key = "\"" + field + "\"";
    size_t key_pos = json.find(search_key);
    
    if (key_pos == std::string::npos) {
        std::cerr << "DEBUG: Field '" << field << "' not found" << std::endl;
        return "";
    }
    
    // Find the colon after the field name
    size_t colon_pos = json.find(':', key_pos);
    if (colon_pos == std::string::npos) {
        return "";
    }
    
    // Skip whitespace after colon
    size_t value_start = colon_pos + 1;
    while (value_start < json.length() && (json[value_start] == ' ' || json[value_start] == '\t')) {
        value_start++;
    }
    
    if (value_start >= json.length()) {
        return "";
    }
    
    // Check if value is a string (starts with quote)
    if (json[value_start] == '"') {
        value_start++; // Skip opening quote
        size_t value_end = value_start;
        
        // Find closing quote (handle escaped quotes)
        while (value_end < json.length() && json[value_end] != '"') {
            if (json[value_end] == '\\' && value_end + 1 < json.length()) {
                value_end += 2; // Skip escaped character
            } else {
                value_end++;
            }
        }
        
        if (value_end >= json.length()) {
            return "";
        }
        
        std::string result = json.substr(value_start, value_end - value_start);
        std::cerr << "DEBUG: extractJsonField - Extracted '" << field << "' = '" << result << "'" << std::endl;
        return result;
    }
    
    // Check if value is a number
    if (std::isdigit(json[value_start]) || json[value_start] == '-') {
        size_t value_end = value_start;
        while (value_end < json.length() && std::isdigit(json[value_end])) {
            value_end++;
        }
        std::string result = json.substr(value_start, value_end - value_start);
        std::cerr << "DEBUG: extractJsonField - Extracted numeric '" << field << "' = '" << result << "'" << std::endl;
        return result;
    }
    
    std::cerr << "DEBUG: extractJsonField - Could not parse field '" << field << "'" << std::endl;
    return "";
}

bool AcademicSocialServer::validateUserRegistration(const std::string& username,
                                                    const std::string& email,
                                                    const std::string& password,
                                                    std::string& error) {
    if (username.empty() || !User::isValidUsername(username)) {
        error = "Username must be 3-32 characters, alphanumeric or underscore";
        return false;
    }
    if (email.empty() || !User::isValidEmail(email)) {
        error = "Email must contain @ symbol";
        return false;
    }
    if (password.empty() || !User::isValidPassword(password)) {
        error = "Password must be at least 8 characters long";
        return false;
    }
    return true;
}

void AcademicSocialServer::ensureDemoUserExists() {
    // Helper lambda to assign Admin role to a user
    auto assignAdminRole = [this](int user_id) -> void {
        auto admin_role = role_repository_->findByName("Admin");
        if (admin_role.has_value()) {
            // assignRoleToUser uses INSERT OR IGNORE, so it won't create duplicates
            role_repository_->assignRoleToUser(user_id, admin_role->getId().value());
            std::cout << "Demo user ensured to have Admin permissions" << std::endl;
        } else {
            std::cerr << "Warning: Could not find Admin role for demo user" << std::endl;
        }
    };
    
    auto professor_role = role_repository_->findByName("Professor");
    auto ensureProfessorPrivileges = [this, &professor_role](int user_id) -> void {
        if (professor_role.has_value()) {
            if (!role_repository_->assignRoleToUser(user_id, professor_role->getId().value())) {
                std::cerr << "Warning: Failed to ensure Professor role assignment for demo user" << std::endl;
            }
        } else {
            std::cerr << "Warning: Could not find Professor role for demo user" << std::endl;
        }

        db::Statement stmt(*database_, "UPDATE users SET role = ?, position = ? WHERE id = ?");
        if (!stmt.isValid()) {
            std::cerr << "Warning: Failed to prepare Professor role update for demo user" << std::endl;
            return;
        }

        stmt.bindText(1, "Professor");
        stmt.bindText(2, "Professor");
        stmt.bindInt(3, user_id);

        if (stmt.step() != SQLITE_DONE) {
            std::cerr << "Warning: Failed to persist Professor role for demo user" << std::endl;
        } else {
            std::cout << "Demo user flagged as Professor for primary role" << std::endl;
        }
    };

    // Check if demo user already exists
    auto existing_user = user_repository_->findByUsername("demo_student");
    if (existing_user.has_value()) {
        int demo_user_id = existing_user->getId().value();
        std::cout << "Demo user already exists (ID: " << demo_user_id << ")" << std::endl;
        
        // Always reset the demo user's password to ensure it works after any API changes
        if (user_repository_->updatePassword(demo_user_id, "demo123")) {
            std::cout << "Demo user password reset successfully" << std::endl;
        } else {
            std::cerr << "Warning: Failed to reset demo user password" << std::endl;
        }
        
        assignAdminRole(demo_user_id);
        ensureProfessorPrivileges(demo_user_id);
        return;
    }

    // Create demo user
    User demo_user("demo_student", "demo@example.edu");
    demo_user.setUniversity("Demo University");
    demo_user.setDepartment("Computer Science");
    demo_user.setEnrollmentYear(2023);
    demo_user.setPrimaryLanguage("Turkish");
    demo_user.setPosition(std::string("Professor"));
    demo_user.setRole(std::string("Professor"));

    auto created_user = user_repository_->create(demo_user, "demo123");
    if (created_user.has_value()) {
        int demo_user_id = created_user->getId().value();
        std::cout << "Demo user created successfully (ID: " << demo_user_id << ")" << std::endl;
        assignAdminRole(demo_user_id);
        ensureProfessorPrivileges(demo_user_id);
    } else {
        std::cerr << "Warning: Failed to create demo user" << std::endl;
    }
}

void AcademicSocialServer::ensureSecondDemoUserExists() {
    // Helper lambda to assign Admin role to a user
    auto assignAdminRole = [this](int user_id) -> void {
        auto admin_role = role_repository_->findByName("Admin");
        if (admin_role.has_value()) {
            // assignRoleToUser uses INSERT OR IGNORE, so it won't create duplicates
            role_repository_->assignRoleToUser(user_id, admin_role->getId().value());
            std::cout << "Second demo user ensured to have Admin permissions" << std::endl;
        } else {
            std::cerr << "Warning: Could not find Admin role for second demo user" << std::endl;
        }
    };

    auto professor_role = role_repository_->findByName("Professor");
    auto ensureProfessorPrivileges = [this, &professor_role](int user_id) -> void {
        if (professor_role.has_value()) {
            if (!role_repository_->assignRoleToUser(user_id, professor_role->getId().value())) {
                std::cerr << "Warning: Failed to ensure Professor role assignment for second demo user" << std::endl;
            }
        } else {
            std::cerr << "Warning: Could not find Professor role for second demo user" << std::endl;
        }

        db::Statement stmt(*database_, "UPDATE users SET role = ?, position = ? WHERE id = ?");
        if (!stmt.isValid()) {
            std::cerr << "Warning: Failed to prepare Professor role update for second demo user" << std::endl;
            return;
        }

        stmt.bindText(1, "Professor");
        stmt.bindText(2, "Professor");
        stmt.bindInt(3, user_id);

        if (stmt.step() != SQLITE_DONE) {
            std::cerr << "Warning: Failed to persist Professor role for second demo user" << std::endl;
        } else {
            std::cout << "Second demo user flagged as Professor for primary role" << std::endl;
        }
    };

    // Check if second demo user already exists
    auto existing_user = user_repository_->findByUsername("demo_teacher");
    if (existing_user.has_value()) {
        int demo_user_id = existing_user->getId().value();
        std::cout << "Second demo user already exists (ID: " << demo_user_id << ")" << std::endl;

        // Always reset the demo user's password to ensure it works after any API changes
        if (user_repository_->updatePassword(demo_user_id, "demo123")) {
            std::cout << "Second demo user password reset successfully" << std::endl;
        } else {
            std::cerr << "Warning: Failed to reset second demo user password" << std::endl;
        }

        assignAdminRole(demo_user_id);
        ensureProfessorPrivileges(demo_user_id);
        return;
    }

    // Create second demo user
    User demo_user("demo_teacher", "demo2@example.edu");
    demo_user.setUniversity("Demo University");
    demo_user.setDepartment("Computer Science");
    demo_user.setEnrollmentYear(2023);
    demo_user.setPrimaryLanguage("Turkish");
    demo_user.setPosition(std::string("Professor"));
    demo_user.setRole(std::string("Professor"));

    auto created_user = user_repository_->create(demo_user, "demo123");
    if (created_user.has_value()) {
        int demo_user_id = created_user->getId().value();
        std::cout << "Second demo user created successfully (ID: " << demo_user_id << ")" << std::endl;
        assignAdminRole(demo_user_id);
        ensureProfessorPrivileges(demo_user_id);
    } else {
        std::cerr << "Warning: Failed to create second demo user" << std::endl;
    }
}

// -------------------- Media Endpoints --------------------

HttpResponse AcademicSocialServer::handleUploadMedia(const HttpRequest& request) {
    // Check Content-Type for multipart/form-data
    auto content_type_it = request.headers.find("Content-Type");
    if (content_type_it == request.headers.end()) {
        return createErrorResponse(400, "Content-Type header is required");
    }
    
    std::string content_type = content_type_it->second;
    if (content_type.find("multipart/form-data") == std::string::npos) {
        return createErrorResponse(400, "Content-Type must be multipart/form-data");
    }
    
    // Extract boundary
    auto boundary_opt = utils::MultipartParser::extractBoundary(content_type);
    if (!boundary_opt.has_value()) {
        return createErrorResponse(400, "Missing boundary in Content-Type");
    }
    
    // Parse multipart data
    auto parts = utils::MultipartParser::parse(request.body, boundary_opt.value());
    
    // Validate required fields
    if (parts.find("file") == parts.end()) {
        return createErrorResponse(400, "Missing 'file' field");
    }
    
    if (parts.find("media_type") == parts.end()) {
        return createErrorResponse(400, "Missing 'media_type' field");
    }
    
    if (parts.find("user_id") == parts.end()) {
        return createErrorResponse(400, "Missing 'user_id' field");
    }
    
    // Extract fields
    auto& file_part = parts["file"];
    std::string media_type_str(parts["media_type"].data.begin(), parts["media_type"].data.end());
    std::string user_id_str(parts["user_id"].data.begin(), parts["user_id"].data.end());
    
    // Parse user_id
    int user_id;
    try {
        user_id = std::stoi(user_id_str);
    } catch (...) {
        return createErrorResponse(400, "Invalid user_id");
    }
    
    // Validate file type (allow common image types for now)
    std::vector<std::string> allowed_types = {
        "image/jpeg", "image/jpg", "image/png", "image/gif", "image/webp"
    };
    
    if (!services::StorageService::validateFileType(file_part.content_type, allowed_types)) {
        return createErrorResponse(400, "Invalid file type. Allowed: JPEG, PNG, GIF, WebP");
    }
    
    // Validate file size (5MB max)
    const size_t MAX_FILE_SIZE = 5 * 1024 * 1024;
    if (!services::StorageService::validateFileSize(file_part.data.size(), MAX_FILE_SIZE)) {
        return createErrorResponse(400, "File too large. Maximum size: 5MB");
    }
    
    // Store file
    auto metadata_opt = storage_service_->storeFile(
        file_part.data,
        file_part.filename,
        file_part.content_type,
        user_id,
        media_type_str
    );
    
    if (!metadata_opt.has_value()) {
        return createErrorResponse(500, "Failed to store file");
    }
    
    auto& metadata = metadata_opt.value();
    
    // Create media record in database
    Media media(user_id, media_type_str, metadata.storage_key);
    media.setFileName(metadata.file_name);
    media.setFileSize(static_cast<int>(metadata.file_size));
    media.setMimeType(metadata.mime_type);
    media.setUrl(metadata.url);
    
    auto created_media = media_repository_->create(media);
    if (!created_media.has_value()) {
        // Cleanup stored file
        storage_service_->deleteFile(metadata.storage_key);
        return createErrorResponse(500, "Failed to create media record");
    }
    
    return createJsonResponse(201, created_media->toJson());
}

HttpResponse AcademicSocialServer::handleGetMediaFile(const HttpRequest& request) {
    // Extract storage key from path: /api/media/file/{storage_key}
    std::string prefix = "/api/media/file/";
    if (request.path.find(prefix) != 0) {
        return createErrorResponse(404, "Invalid path");
    }
    
    std::string storage_key = request.path.substr(prefix.length());
    
    // Remove query string if present
    size_t query_pos = storage_key.find('?');
    if (query_pos != std::string::npos) {
        storage_key = storage_key.substr(0, query_pos);
    }
    
    // Retrieve file
    auto file_data = storage_service_->retrieveFile(storage_key);
    if (!file_data.has_value()) {
        return createErrorResponse(404, "File not found");
    }
    
    // Determine content type from storage key extension
    std::string content_type = "application/octet-stream";
    if (storage_key.find(".jpg") != std::string::npos || storage_key.find(".jpeg") != std::string::npos) {
        content_type = "image/jpeg";
    } else if (storage_key.find(".png") != std::string::npos) {
        content_type = "image/png";
    } else if (storage_key.find(".gif") != std::string::npos) {
        content_type = "image/gif";
    } else if (storage_key.find(".webp") != std::string::npos) {
        content_type = "image/webp";
    }
    
    // Convert binary data to string for response
    std::string body(file_data->begin(), file_data->end());
    
    return HttpResponse(200, content_type, body);
}

HttpResponse AcademicSocialServer::handleGetUserMedia(const HttpRequest& request) {
    // Extract user ID from path: /api/users/{id}/media
    std::regex user_media_regex("/api/users/(\\d+)/media");
    std::smatch match;
    
    if (!std::regex_match(request.path, match, user_media_regex)) {
        return createErrorResponse(404, "Invalid path");
    }
    
    int user_id = std::stoi(match[1].str());
    
    // Get user's media
    auto media_list = media_repository_->findByUser(user_id);
    
    // Build JSON array
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < media_list.size(); ++i) {
        if (i > 0) oss << ",";
        oss << media_list[i].toJson();
    }
    oss << "]";
    
    return createJsonResponse(200, oss.str());
}

// ==================== Helper Methods ====================

// COMPLETE getUserIdFromAuth function - Replace your entire function with this:

int AcademicSocialServer::getUserIdFromAuth(const HttpRequest& request) {
    // Extract user ID from Authorization header (JWT token)
    // Try different cases of Authorization header
    auto it = request.headers.find("Authorization");
    if (it == request.headers.end()) {
        it = request.headers.find("authorization");
    }
    if (it != request.headers.end()) {
        std::string auth_header = it->second;
        std::cerr << "DEBUG: Authorization header found, length: " << auth_header.length() << std::endl;
        
        if (auth_header.find("Bearer ") == 0) {
            std::string token = auth_header.substr(7);
            std::cerr << "DEBUG: Extracted Bearer token (first 50 chars): " << token.substr(0, 50) << "..." << std::endl;
            
            // Verify and decode JWT token
            try {
                std::string jwt_secret = config::get_jwt_secret();
                std::cerr << "DEBUG: JWT secret loaded, length: " << jwt_secret.length() << std::endl;
                
                auto payload = security::verify_jwt_token(token, jwt_secret);
                
                if (payload.has_value()) {
                    std::cerr << "DEBUG: JWT verified successfully, user_id: " << payload->user_id << std::endl;
                    return payload->user_id;
                } else {
                    std::cerr << "DEBUG: JWT verification returned empty payload (token invalid or expired)" << std::endl;
                    return -1;
                }
            } catch (const std::exception& e) {
                std::cerr << "JWT verification error: " << e.what() << std::endl;
                std::cerr << "DEBUG: Token was: " << token.substr(0, 100) << "..." << std::endl;
                return -1;
            }
        } else {
            std::cerr << "DEBUG: Authorization header doesn't start with 'Bearer '" << std::endl;
            std::cerr << "DEBUG: Header value: " << auth_header.substr(0, 50) << std::endl;
        }
    } else {
        std::cerr << "DEBUG: No Authorization header found" << std::endl;
    }
    
    // Fallback: check for X-User-ID header (for testing)
    it = request.headers.find("X-User-ID");
    if (it != request.headers.end()) {
        std::cerr << "DEBUG: Using X-User-ID fallback header: " << it->second << std::endl;
        try {
            return std::stoi(it->second);
        } catch (...) {
            return -1;
        }
    }
    
    // Check for demo user in request body (for demo/testing purposes only)
    std::string username = extractJsonField(request.body, "username");
    if (username == "demo_student") {
        std::cerr << "DEBUG: Using demo_student fallback from body" << std::endl;
        auto demo_user = user_repository_->findByUsername("demo_student");
        if (demo_user.has_value() && demo_user->getId().has_value()) {
            return demo_user->getId().value();
        }
    }
    
    std::cerr << "DEBUG: All authentication methods failed, returning -1" << std::endl;
    return -1;
}

int AcademicSocialServer::extractIdFromPath(const std::string& path, const std::string& prefix) {
    size_t pos = path.find(prefix);
    if (pos == std::string::npos) return -1;
    
    std::string after_prefix = path.substr(pos + prefix.length());
    size_t end = after_prefix.find('/');
    std::string id_str = (end == std::string::npos) ? after_prefix : after_prefix.substr(0, end);
    
    try {
        return std::stoi(id_str);
    } catch (...) {
        return -1;
    }
}

// ==================== Friendship Handlers ====================

HttpResponse AcademicSocialServer::handleCreateFriendship(const HttpRequest& request) {
    int requester_id = getUserIdFromAuth(request);
    if (requester_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    std::string addressee_id_str = extractJsonField(request.body, "addressee_id");
    if (addressee_id_str.empty()) {
        return createErrorResponse(400, "addressee_id is required");
    }
    
    int addressee_id = std::stoi(addressee_id_str);
    
    // Check if friendship already exists
    auto existing = friendship_repository_->findBetweenUsers(requester_id, addressee_id);
    if (existing.has_value()) {
        return createErrorResponse(409, "Friendship request already exists");
    }
    
    Friendship friendship(requester_id, addressee_id);
    auto created = friendship_repository_->create(friendship);
    
    if (created.has_value()) {
        return createJsonResponse(201, created->toJson());
    }
    
    return createErrorResponse(500, "Failed to create friendship request");
}

HttpResponse AcademicSocialServer::handleGetFriendships(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    // Parse query parameters for status filter
    std::string status = "";
    std::regex status_regex("[?&]status=([^&]*)");
    std::smatch match;
    if (std::regex_search(request.path, match, status_regex)) {
        status = match[1].str();
    }
    
    std::vector<Friendship> friendships;
    if (status == "pending") {
        // Get incoming pending requests
        friendships = friendship_repository_->findPendingRequestsForUser(user_id);
    } else if (status == "sent") {
        // Get outgoing pending requests
        friendships = friendship_repository_->findSentRequestsByUser(user_id);
    } else {
        // Get all friendships for user
        friendships = friendship_repository_->findByUserId(user_id, status);
    }
    
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < friendships.size(); ++i) {
        oss << friendships[i].toJson();
        if (i < friendships.size() - 1) oss << ",";
    }
    oss << "]";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetFriends(const HttpRequest& request) {
    int user_id = extractIdFromPath(request.path, "/api/users/");
    if (user_id < 0) {
        return createErrorResponse(400, "Invalid user ID");
    }
    
    std::vector<User> friends = friendship_repository_->getFriendsForUser(user_id);
    
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < friends.size(); ++i) {
        oss << friends[i].toJson();
        if (i < friends.size() - 1) oss << ",";
    }
    oss << "]";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleAcceptFriendship(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int friendship_id = extractIdFromPath(request.path, "/api/friendships/");
    if (friendship_id < 0) {
        return createErrorResponse(400, "Invalid friendship ID");
    }
    
    // Verify the user is the addressee
    auto friendship = friendship_repository_->findById(friendship_id);
    if (!friendship.has_value()) {
        return createErrorResponse(404, "Friendship request not found");
    }
    
    if (friendship->getAddresseeId() != user_id) {
        return createErrorResponse(403, "You can only accept requests sent to you");
    }
    
    if (friendship_repository_->acceptRequest(friendship_id)) {
        auto updated = friendship_repository_->findById(friendship_id);
        return createJsonResponse(200, updated->toJson());
    }
    
    return createErrorResponse(500, "Failed to accept friendship request");
}

HttpResponse AcademicSocialServer::handleRejectFriendship(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int friendship_id = extractIdFromPath(request.path, "/api/friendships/");
    if (friendship_id < 0) {
        return createErrorResponse(400, "Invalid friendship ID");
    }
    
    // Verify the user is the addressee
    auto friendship = friendship_repository_->findById(friendship_id);
    if (!friendship.has_value()) {
        return createErrorResponse(404, "Friendship request not found");
    }
    
    if (friendship->getAddresseeId() != user_id) {
        return createErrorResponse(403, "You can only reject requests sent to you");
    }
    
    if (friendship_repository_->rejectRequest(friendship_id)) {
        auto updated = friendship_repository_->findById(friendship_id);
        return createJsonResponse(200, updated->toJson());
    }
    
    return createErrorResponse(500, "Failed to reject friendship request");
}

HttpResponse AcademicSocialServer::handleDeleteFriendship(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int friendship_id = extractIdFromPath(request.path, "/api/friendships/");
    if (friendship_id < 0) {
        return createErrorResponse(400, "Invalid friendship ID");
    }
    
    // Verify the user is part of the friendship
    auto friendship = friendship_repository_->findById(friendship_id);
    if (!friendship.has_value()) {
        return createErrorResponse(404, "Friendship not found");
    }
    
    // Allow deleting if user is part of the friendship OR has delete_any_friendship permission
    if (friendship->getRequesterId() != user_id && friendship->getAddresseeId() != user_id && !role_repository_->userHasPermission(user_id, "delete_any_friendship")) {
        return createErrorResponse(403, "You can only delete your own friendships");
    }
    
    if (friendship_repository_->deleteById(friendship_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to delete friendship");
}

// ==================== Post Handlers ====================

HttpResponse AcademicSocialServer::handleCreatePost(const HttpRequest& request) {
    int author_id = getUserIdFromAuth(request);
    if (author_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (content.empty()) {
        return createErrorResponse(400, "content is required");
    }
    
    Post post(author_id, content);
    
    std::string visibility = extractJsonField(request.body, "visibility");
    if (!visibility.empty()) {
        post.setVisibility(visibility);
    }
    
    std::string media_urls = extractJsonField(request.body, "media_urls");
    if (!media_urls.empty()) {
        post.setMediaUrls(media_urls);
    }
    
    std::string group_id_str = extractJsonField(request.body, "group_id");
    if (!group_id_str.empty()) {
        post.setGroupId(std::stoi(group_id_str));
    }
    
    auto created = post_repository_->create(post);
    if (created.has_value()) {
        int post_id = created->getId().value();

        // Extract and save hashtags
        auto hashtags = utils::TextParser::extractHashtags(content);
        if (!hashtags.empty()) {
            auto hashtag_records = hashtag_repository_->findOrCreateTags(hashtags);
            std::vector<int> hashtag_ids;
            for (const auto& tag : hashtag_records) {
                if (tag.getId().has_value()) {
                    hashtag_ids.push_back(tag.getId().value());
                }
            }
            hashtag_repository_->linkTagsToPost(hashtag_ids, post_id);
        }

        // Populate author information from user repository
        auto author = user_repository_->findById(author_id);

        // Extract and save mentions
        auto mentions = utils::TextParser::extractMentions(content);
        if (!mentions.empty() && author.has_value()) {
            std::set<int> mentioned_user_ids;
            for (const auto& username : mentions) {
                auto user = user_repository_->findByUsername(username);
                if (user.has_value() && user->getId().has_value()) {
                    int mentioned_id = user->getId().value();
                    mentioned_user_ids.insert(mentioned_id);

                    // Create notification for mention (only if not mentioning self)
                    if (mentioned_id != author_id) {
                        notification_repository_->createNotification(
                            mentioned_id,
                            "mention",
                            "You were mentioned in a post",
                            author->getUsername() + " mentioned you in a post",
                            author_id,
                            post_id,
                            std::nullopt,  // comment_id
                            std::nullopt,  // group_id
                            std::nullopt,  // session_id
                            "/posts/" + std::to_string(post_id)
                        );
                    }
                }
            }
            mention_repository_->createMentions(post_id, mentioned_user_ids);
        }

        // Set author information
        if (author.has_value()) {
            created->setAuthorUsername(author->getUsername());
            if (author->getName().has_value()) {
                created->setAuthorName(author->getName().value());
            }
            if (author->getAvatarUrl().has_value()) {
                created->setAuthorAvatarUrl(author->getAvatarUrl().value());
            }
        }
        return createJsonResponse(201, created->toJson());
    }

    return createErrorResponse(500, "Failed to create post");
}

HttpResponse AcademicSocialServer::handleGetPosts(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    // Parse pagination parameters
    int limit = 50;
    int offset = 0;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }
    
    std::vector<Post> posts = post_repository_->findFeedForUser(user_id, limit, offset);

    std::ostringstream oss;
    oss << "{\"posts\":[";
    for (size_t i = 0; i < posts.size(); ++i) {
        oss << posts[i].toJson();
        if (i < posts.size() - 1) oss << ",";
    }
    oss << "],\"total\":" << posts.size() << "}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetUserPosts(const HttpRequest& request) {
    int user_id = extractIdFromPath(request.path, "/api/users/");
    if (user_id < 0) {
        return createErrorResponse(400, "Invalid user ID");
    }
    
    // Parse pagination parameters
    int limit = 50;
    int offset = 0;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }
    
    std::vector<Post> posts = post_repository_->findByAuthor(user_id, limit, offset);
    
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < posts.size(); ++i) {
        oss << posts[i].toJson();
        if (i < posts.size() - 1) oss << ",";
    }
    oss << "]";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleUpdatePost(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    auto post = post_repository_->findById(post_id);
    if (!post.has_value()) {
        return createErrorResponse(404, "Post not found");
    }
    
    // Allow editing if user owns the post OR has edit_any_post permission
    if (post->getAuthorId() != user_id && !role_repository_->userHasPermission(user_id, "edit_any_post")) {
        return createErrorResponse(403, "You can only edit your own posts");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (!content.empty()) {
        post->setContent(content);
    }
    
    std::string visibility = extractJsonField(request.body, "visibility");
    if (!visibility.empty()) {
        post->setVisibility(visibility);
    }
    
    if (post_repository_->update(post.value())) {
        auto updated = post_repository_->findById(post_id);
        return createJsonResponse(200, updated->toJson());
    }
    
    return createErrorResponse(500, "Failed to update post");
}

HttpResponse AcademicSocialServer::handleDeletePost(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    auto post = post_repository_->findById(post_id);
    if (!post.has_value()) {
        return createErrorResponse(404, "Post not found");
    }
    
    // Allow deleting if user owns the post OR has delete_any_post permission
    if (post->getAuthorId() != user_id && !role_repository_->userHasPermission(user_id, "delete_any_post")) {
        return createErrorResponse(403, "You can only delete your own posts");
    }
    
    if (post_repository_->deleteById(post_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to delete post");
}

HttpResponse AcademicSocialServer::handleAddReaction(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    std::string reaction_type = extractJsonField(request.body, "reaction_type");
    if (reaction_type.empty()) {
        reaction_type = "like"; // Default reaction type
    }
    
    if (post_repository_->addReaction(post_id, user_id, reaction_type)) {
        std::ostringstream oss;
        oss << "{\"success\":true,\"reaction_type\":\"" << reaction_type << "\"}";
        return createJsonResponse(200, oss.str());
    }
    
    return createErrorResponse(500, "Failed to add reaction");
}

HttpResponse AcademicSocialServer::handleRemoveReaction(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    // Extract reaction type from query parameters or body
    std::string reaction_type = "like"; // Default
    std::regex reaction_regex("[?&]reaction_type=([^&]*)");
    std::smatch match;
    if (std::regex_search(request.path, match, reaction_regex)) {
        reaction_type = match[1].str();
    }
    
    if (post_repository_->removeReaction(post_id, user_id, reaction_type)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to remove reaction");
}

// ==================== Comment Handlers ====================

HttpResponse AcademicSocialServer::handleCreateComment(const HttpRequest& request) {
    int author_id = getUserIdFromAuth(request);
    if (author_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (content.empty()) {
        return createErrorResponse(400, "content is required");
    }
    
    Comment comment(post_id, author_id, content);
    
    auto created = comment_repository_->create(comment);
    if (created.has_value()) {
        return createJsonResponse(201, created->toJson());
    }
    
    return createErrorResponse(500, "Failed to create comment");
}

HttpResponse AcademicSocialServer::handleGetComments(const HttpRequest& request) {
    int post_id = extractIdFromPath(request.path, "/api/posts/");
    if (post_id < 0) {
        return createErrorResponse(400, "Invalid post ID");
    }
    
    // Parse pagination parameters
    int limit = 100;
    int offset = 0;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }
    
    std::vector<Comment> comments = comment_repository_->findByPostId(post_id, limit, offset);
    
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < comments.size(); ++i) {
        oss << comments[i].toJson();
        if (i < comments.size() - 1) oss << ",";
    }
    oss << "]";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleReplyToComment(const HttpRequest& request) {
    int author_id = getUserIdFromAuth(request);
    if (author_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int parent_comment_id = extractIdFromPath(request.path, "/api/comments/");
    if (parent_comment_id < 0) {
        return createErrorResponse(400, "Invalid comment ID");
    }
    
    // Get the parent comment to extract post_id
    auto parent_comment = comment_repository_->findById(parent_comment_id);
    if (!parent_comment.has_value()) {
        return createErrorResponse(404, "Parent comment not found");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (content.empty()) {
        return createErrorResponse(400, "content is required");
    }
    
    Comment comment(parent_comment->getPostId(), author_id, content);
    comment.setParentId(parent_comment_id);
    
    auto created = comment_repository_->create(comment);
    if (created.has_value()) {
        return createJsonResponse(201, created->toJson());
    }
    
    return createErrorResponse(500, "Failed to create reply");
}

HttpResponse AcademicSocialServer::handleUpdateComment(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int comment_id = extractIdFromPath(request.path, "/api/comments/");
    if (comment_id < 0) {
        return createErrorResponse(400, "Invalid comment ID");
    }
    
    auto comment = comment_repository_->findById(comment_id);
    if (!comment.has_value()) {
        return createErrorResponse(404, "Comment not found");
    }
    
    // Allow editing if user owns the comment OR has edit_any_comment permission
    if (comment->getAuthorId() != user_id && !role_repository_->userHasPermission(user_id, "edit_any_comment")) {
        return createErrorResponse(403, "You can only edit your own comments");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (!content.empty()) {
        comment->setContent(content);
    }
    
    if (comment_repository_->update(comment.value())) {
        auto updated = comment_repository_->findById(comment_id);
        return createJsonResponse(200, updated->toJson());
    }
    
    return createErrorResponse(500, "Failed to update comment");
}

HttpResponse AcademicSocialServer::handleDeleteComment(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int comment_id = extractIdFromPath(request.path, "/api/comments/");
    if (comment_id < 0) {
        return createErrorResponse(400, "Invalid comment ID");
    }
    
    auto comment = comment_repository_->findById(comment_id);
    if (!comment.has_value()) {
        return createErrorResponse(404, "Comment not found");
    }
    
    // Allow deleting if user owns the comment OR has delete_any_comment permission
    if (comment->getAuthorId() != user_id && !role_repository_->userHasPermission(user_id, "delete_any_comment")) {
        return createErrorResponse(403, "You can only delete your own comments");
    }
    
    if (comment_repository_->deleteById(comment_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to delete comment");
}

// Group handlers
HttpResponse AcademicSocialServer::handleCreateGroup(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    // Check if user has permission to create groups (Professor or Admin)
    if (!role_repository_->userHasPermission(user_id, "create_group")) {
        return createErrorResponse(403, "Only professors and admins can create groups");
    }
    
    std::string name = extractJsonField(request.body, "name");
    std::string description = extractJsonField(request.body, "description");
    std::string privacy = extractJsonField(request.body, "privacy");
    
    if (name.empty()) {
        return createErrorResponse(400, "Group name is required");
    }
    
    if (privacy.empty()) {
        privacy = "private";
    }
    
    Group group(name, user_id);
    if (!description.empty()) {
        group.setDescription(description);
    }
    group.setPrivacy(privacy);
    
    auto created = group_repository_->create(group);
    if (created.has_value()) {
        return createJsonResponse(201, created->toJson());
    }
    
    return createErrorResponse(500, "Failed to create group");
}

HttpResponse AcademicSocialServer::handleGetGroups(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    // Check if filtering by membership
    std::regex my_groups_regex("[?&]my_groups=(true|1)");
    std::smatch match;
    bool my_groups = false;
    if (std::regex_search(request.path, match, my_groups_regex)) {
        my_groups = true;
    }
    
    std::vector<Group> groups;
    if (my_groups) {
        groups = group_repository_->findByMember(user_id);
    } else {
        groups = group_repository_->findAll();
    }

    std::ostringstream oss;
    oss << "{\"groups\":[";
    for (size_t i = 0; i < groups.size(); ++i) {
        if (i > 0) oss << ",";

        // Get the base JSON and remove the closing brace
        std::string base_json = groups[i].toJson();
        base_json.pop_back(); // Remove '}'

        oss << base_json;

        // Add user_role and member_count fields
        if (groups[i].getId().has_value()) {
            int group_id = groups[i].getId().value();
            std::string user_role = group_repository_->getMemberRole(group_id, user_id);
            int member_count = group_repository_->getMemberCount(group_id);

            if (!user_role.empty()) {
                oss << ",\"user_role\":\"" << user_role << "\"";
            }
            oss << ",\"member_count\":" << member_count;
        }

        oss << "}";
    }
    oss << "],\"total\":" << groups.size() << "}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetGroup(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    return createJsonResponse(200, group->toJson());
}

HttpResponse AcademicSocialServer::handleUpdateGroup(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    // Check if user can manage this group
    if (!group_repository_->canUserManage(group_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to update this group");
    }
    
    std::string name = extractJsonField(request.body, "name");
    std::string description = extractJsonField(request.body, "description");
    std::string privacy = extractJsonField(request.body, "privacy");
    
    if (!name.empty()) {
        group->setName(name);
    }
    if (!description.empty()) {
        group->setDescription(description);
    }
    if (!privacy.empty()) {
        group->setPrivacy(privacy);
    }
    
    if (group_repository_->update(*group)) {
        return createJsonResponse(200, group->toJson());
    }
    
    return createErrorResponse(500, "Failed to update group");
}

HttpResponse AcademicSocialServer::handleDeleteGroup(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    // Only creator or admin can delete
    if (group->getCreatorId() != user_id && !role_repository_->userHasPermission(user_id, "manage_users")) {
        return createErrorResponse(403, "Only the group creator or admins can delete groups");
    }
    
    if (group_repository_->deleteById(group_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to delete group");
}

HttpResponse AcademicSocialServer::handleAddGroupMember(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    std::string user_id_str = extractJsonField(request.body, "user_id");
    std::string role = extractJsonField(request.body, "role");
    
    if (user_id_str.empty()) {
        return createErrorResponse(400, "User ID is required");
    }
    
    int member_user_id = std::stoi(user_id_str);
    if (role.empty()) {
        role = "member";
    }
    
    // Check if requester can manage the group
    if (!group_repository_->canUserManage(group_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to add members to this group");
    }
    
    if (group_repository_->addMember(group_id, member_user_id, role)) {
        return createJsonResponse(200, "{\"message\":\"Member added successfully\"}");
    }
    
    return createErrorResponse(500, "Failed to add member");
}

HttpResponse AcademicSocialServer::handleRemoveGroupMember(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    // Extract member user ID from path
    std::regex user_id_regex("/api/groups/\\d+/members/(\\d+)");
    std::smatch match;
    if (!std::regex_search(request.path, match, user_id_regex) || match.size() < 2) {
        return createErrorResponse(400, "Invalid member user ID");
    }
    int member_user_id = std::stoi(match[1].str());
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    // Check if requester can manage the group
    if (!group_repository_->canUserManage(group_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to remove members from this group");
    }
    
    if (group_repository_->removeMember(group_id, member_user_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to remove member");
}

HttpResponse AcademicSocialServer::handleUpdateGroupMemberRole(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int group_id = extractIdFromPath(request.path, "/api/groups/");
    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }
    
    // Extract member user ID from path
    std::regex user_id_regex("/api/groups/\\d+/members/(\\d+)");
    std::smatch match;
    if (!std::regex_search(request.path, match, user_id_regex) || match.size() < 2) {
        return createErrorResponse(400, "Invalid member user ID");
    }
    int member_user_id = std::stoi(match[1].str());
    
    auto group = group_repository_->findById(group_id);
    if (!group.has_value()) {
        return createErrorResponse(404, "Group not found");
    }
    
    // Check if requester can manage the group
    if (!group_repository_->canUserManage(group_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to update member roles");
    }
    
    std::string role = extractJsonField(request.body, "role");
    if (role.empty()) {
        return createErrorResponse(400, "Role is required");
    }
    
    if (group_repository_->updateMemberRole(group_id, member_user_id, role)) {
        return createJsonResponse(200, "{\"message\":\"Member role updated successfully\"}");
    }
    
    return createErrorResponse(500, "Failed to update member role");
}

// Organization handlers
HttpResponse AcademicSocialServer::handleCreateOrganization(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    // Check if user has permission to create organizations
    if (!role_repository_->userHasPermission(user_id, "manage_organizations")) {
        return createErrorResponse(403, "Only admins can create organizations");
    }
    
    std::string name = extractJsonField(request.body, "name");
    std::string type = extractJsonField(request.body, "type");
    std::string description = extractJsonField(request.body, "description");
    std::string email = extractJsonField(request.body, "email");
    std::string website = extractJsonField(request.body, "website");
    
    if (name.empty()) {
        return createErrorResponse(400, "Organization name is required");
    }
    if (type.empty()) {
        return createErrorResponse(400, "Organization type is required");
    }
    
    Organization org(name, type);
    if (!description.empty()) {
        org.setDescription(description);
    }
    if (!email.empty()) {
        org.setEmail(email);
    }
    if (!website.empty()) {
        org.setWebsite(website);
    }
    
    auto created = organization_repository_->create(org);
    if (created.has_value()) {
        // Add creator as owner
        organization_repository_->addAccount(created->getId().value(), user_id, "owner");
        return createJsonResponse(201, created->toJson());
    }
    
    return createErrorResponse(500, "Failed to create organization");
}

HttpResponse AcademicSocialServer::handleGetOrganizations(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    // Parse pagination parameters
    int limit = 100;
    int offset = 0;

    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;

    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }

    std::vector<Organization> orgs = organization_repository_->findAll(limit, offset);

    // Count total organizations for pagination
    // For now, return the size of current result
    // In production, you'd want a separate count query
    int total = orgs.size();

    std::ostringstream oss;
    oss << "{\"organizations\":[";
    for (size_t i = 0; i < orgs.size(); ++i) {
        if (i > 0) oss << ",";
        oss << orgs[i].toJson();
    }
    oss << "],\"total\":" << orgs.size() << "}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetOrganization(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int org_id = extractIdFromPath(request.path, "/api/organizations/");
    if (org_id < 0) {
        return createErrorResponse(400, "Invalid organization ID");
    }
    
    auto org = organization_repository_->findById(org_id);
    if (!org.has_value()) {
        return createErrorResponse(404, "Organization not found");
    }
    
    return createJsonResponse(200, org->toJson());
}

HttpResponse AcademicSocialServer::handleUpdateOrganization(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int org_id = extractIdFromPath(request.path, "/api/organizations/");
    if (org_id < 0) {
        return createErrorResponse(400, "Invalid organization ID");
    }
    
    auto org = organization_repository_->findById(org_id);
    if (!org.has_value()) {
        return createErrorResponse(404, "Organization not found");
    }
    
    // Check if user can manage this organization
    if (!organization_repository_->canUserManage(org_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to update this organization");
    }
    
    std::string name = extractJsonField(request.body, "name");
    std::string type = extractJsonField(request.body, "type");
    std::string description = extractJsonField(request.body, "description");
    std::string email = extractJsonField(request.body, "email");
    std::string website = extractJsonField(request.body, "website");
    
    if (!name.empty()) {
        org->setName(name);
    }
    if (!type.empty()) {
        org->setType(type);
    }
    if (!description.empty()) {
        org->setDescription(description);
    }
    if (!email.empty()) {
        org->setEmail(email);
    }
    if (!website.empty()) {
        org->setWebsite(website);
    }
    
    if (organization_repository_->update(*org)) {
        return createJsonResponse(200, org->toJson());
    }
    
    return createErrorResponse(500, "Failed to update organization");
}

HttpResponse AcademicSocialServer::handleDeleteOrganization(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int org_id = extractIdFromPath(request.path, "/api/organizations/");
    if (org_id < 0) {
        return createErrorResponse(400, "Invalid organization ID");
    }
    
    auto org = organization_repository_->findById(org_id);
    if (!org.has_value()) {
        return createErrorResponse(404, "Organization not found");
    }
    
    // Only admins can delete organizations
    if (!role_repository_->userHasPermission(user_id, "manage_organizations")) {
        return createErrorResponse(403, "Only admins can delete organizations");
    }
    
    if (organization_repository_->deleteById(org_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to delete organization");
}

HttpResponse AcademicSocialServer::handleAddOrganizationAccount(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int org_id = extractIdFromPath(request.path, "/api/organizations/");
    if (org_id < 0) {
        return createErrorResponse(400, "Invalid organization ID");
    }
    
    auto org = organization_repository_->findById(org_id);
    if (!org.has_value()) {
        return createErrorResponse(404, "Organization not found");
    }
    
    std::string account_user_id_str = extractJsonField(request.body, "user_id");
    std::string role = extractJsonField(request.body, "role");
    
    if (account_user_id_str.empty()) {
        return createErrorResponse(400, "User ID is required");
    }
    
    int account_user_id = std::stoi(account_user_id_str);
    if (role.empty()) {
        role = "editor";
    }
    
    // Check if requester can manage the organization
    if (!organization_repository_->canUserManage(org_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to add accounts to this organization");
    }
    
    if (organization_repository_->addAccount(org_id, account_user_id, role)) {
        return createJsonResponse(200, "{\"message\":\"Account added successfully\"}");
    }
    
    return createErrorResponse(500, "Failed to add account");
}

HttpResponse AcademicSocialServer::handleRemoveOrganizationAccount(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int org_id = extractIdFromPath(request.path, "/api/organizations/");
    if (org_id < 0) {
        return createErrorResponse(400, "Invalid organization ID");
    }
    
    // Extract account user ID from path
    std::regex account_id_regex("/api/organizations/\\d+/accounts/(\\d+)");
    std::smatch match;
    if (!std::regex_search(request.path, match, account_id_regex) || match.size() < 2) {
        return createErrorResponse(400, "Invalid account user ID");
    }
    int account_user_id = std::stoi(match[1].str());
    
    auto org = organization_repository_->findById(org_id);
    if (!org.has_value()) {
        return createErrorResponse(404, "Organization not found");
    }
    
    // Check if requester can manage the organization
    if (!organization_repository_->canUserManage(org_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to remove accounts from this organization");
    }
    
    if (organization_repository_->removeAccount(org_id, account_user_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to remove account");
}

// ============================================================================
// Chat/Messaging Handlers
// ============================================================================

HttpResponse AcademicSocialServer::handleGetConversations(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    auto conversations = conversation_repository_->getUserConversations(user_id);
    
    std::ostringstream oss;
    oss << "{\"conversations\":[";
    
    for (size_t i = 0; i < conversations.size(); ++i) {
        oss << conversations[i].to_json();
        if (i < conversations.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "],\"count\":" << conversations.size() << "}";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetOrCreateConversation(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    std::string other_user_id_str = extractJsonField(request.body, "user_id");
    if (other_user_id_str.empty()) {
        return createErrorResponse(400, "Missing user_id field");
    }
    
    int other_user_id;
    try {
        other_user_id = std::stoi(other_user_id_str);
    } catch (const std::exception&) {
        return createErrorResponse(400, "Invalid user_id");
    }
    
    if (other_user_id == user_id) {
        return createErrorResponse(400, "Cannot create conversation with yourself");
    }
    
    // Check if other user exists
    auto other_user = user_repository_->findById(other_user_id);
    if (!other_user.has_value()) {
        return createErrorResponse(404, "User not found");
    }
    
    auto conversation = conversation_repository_->findOrCreateConversation(user_id, other_user_id);
    if (!conversation.has_value()) {
        return createErrorResponse(500, "Failed to create conversation");
    }
    
    return createJsonResponse(200, conversation->to_json());
}

HttpResponse AcademicSocialServer::handleGetMessages(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int conversation_id = extractIdFromPath(request.path, "/api/conversations/");
    if (conversation_id < 0) {
        return createErrorResponse(400, "Invalid conversation ID");
    }
    
    // Check if user is part of this conversation
    auto conversation = conversation_repository_->getById(conversation_id);
    if (!conversation.has_value()) {
        return createErrorResponse(404, "Conversation not found");
    }
    
    if (conversation->user1_id != user_id && conversation->user2_id != user_id) {
        return createErrorResponse(403, "You don't have access to this conversation");
    }
    
    // Parse pagination parameters
    int limit = 50;
    int offset = 0;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        int parsed_limit = std::stoi(match[1].str());
        if (parsed_limit > 0 && parsed_limit <= 100) {
            limit = parsed_limit;
        }
    }
    
    if (std::regex_search(request.path, match, offset_regex)) {
        int parsed_offset = std::stoi(match[1].str());
        if (parsed_offset >= 0) {
            offset = parsed_offset;
        }
    }
    
    auto messages = message_repository_->getConversationMessages(conversation_id, limit, offset);
    
    std::ostringstream oss;
    oss << "{\"messages\":[";
    
    for (size_t i = 0; i < messages.size(); ++i) {
        oss << messages[i].to_json();
        if (i < messages.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "],\"count\":" << messages.size();
    oss << ",\"limit\":" << limit;
    oss << ",\"offset\":" << offset << "}";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleSendMessage(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int conversation_id = extractIdFromPath(request.path, "/api/conversations/");
    if (conversation_id < 0) {
        return createErrorResponse(400, "Invalid conversation ID");
    }
    
    // Check if user is part of this conversation
    auto conversation = conversation_repository_->getById(conversation_id);
    if (!conversation.has_value()) {
        return createErrorResponse(404, "Conversation not found");
    }
    
    if (conversation->user1_id != user_id && conversation->user2_id != user_id) {
        return createErrorResponse(403, "You don't have access to this conversation");
    }
    
    std::string content = extractJsonField(request.body, "content");
    if (content.empty()) {
        return createErrorResponse(400, "Message content cannot be empty");
    }
    
    std::string media_url = extractJsonField(request.body, "media_url");
    
    auto message = message_repository_->createMessage(conversation_id, user_id, content, media_url);
    if (!message.has_value()) {
        return createErrorResponse(500, "Failed to send message");
    }
    
    // Update conversation's last_message_at timestamp
    conversation_repository_->updateLastMessageTime(conversation_id);
    
    return createJsonResponse(201, message->to_json());
}

HttpResponse AcademicSocialServer::handleMarkMessageRead(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int message_id = extractIdFromPath(request.path, "/api/messages/");
    if (message_id < 0) {
        return createErrorResponse(400, "Invalid message ID");
    }
    
    auto message = message_repository_->getById(message_id);
    if (!message.has_value()) {
        return createErrorResponse(404, "Message not found");
    }
    
    // Check if user is the recipient (not the sender)
    auto conversation = conversation_repository_->getById(message->conversation_id);
    if (!conversation.has_value()) {
        return createErrorResponse(404, "Conversation not found");
    }
    
    if (conversation->user1_id != user_id && conversation->user2_id != user_id) {
        return createErrorResponse(403, "You don't have access to this conversation");
    }
    
    if (message->sender_id == user_id) {
        return createErrorResponse(400, "Cannot mark your own message as read");
    }
    
    if (message_repository_->markAsRead(message_id)) {
        return HttpResponse(204, "text/plain", "");
    }
    
    return createErrorResponse(500, "Failed to mark message as read");
}

// WebSocket handler setup
void AcademicSocialServer::setupWebSocketHandlers() {
    // Handler for chat messages
    websocket_server_->registerHandler("chat:send",
        [this](int user_id, const WebSocketMessage& message) {
            handleChatMessage(user_id, message);
        });

    // Handler for typing indicators
    websocket_server_->registerHandler("chat:typing",
        [this](int user_id, const WebSocketMessage& message) {
            handleTypingIndicator(user_id, message);
        });

    // Voice channel handlers
    websocket_server_->registerHandler("voice:join",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceJoin(user_id, message);
        });

    websocket_server_->registerHandler("voice:leave",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceLeave(user_id, message);
        });

    websocket_server_->registerHandler("voice:offer",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceOffer(user_id, message);
        });

    websocket_server_->registerHandler("voice:answer",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceAnswer(user_id, message);
        });

    websocket_server_->registerHandler("voice:ice-candidate",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceIceCandidate(user_id, message);
        });

    websocket_server_->registerHandler("voice:mute",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceMute(user_id, message);
        });

    websocket_server_->registerHandler("voice:video-toggle",
        [this](int user_id, const WebSocketMessage& message) {
            handleVoiceVideoToggle(user_id, message);
        });

    // Register disconnect handler to cleanup voice sessions
    websocket_server_->registerDisconnectHandler(
        [this](int user_id) {
            handleUserDisconnect(user_id);
        });
}

void AcademicSocialServer::handleChatMessage(int user_id, const WebSocketMessage& message) {
    // Parse payload to extract conversation_id and content
    std::string payload = message.payload;
    
    // Extract conversation_id
    std::regex conv_regex("\"conversation_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;
    int conversation_id = 0;
    if (std::regex_search(payload, matches, conv_regex)) {
        conversation_id = std::stoi(matches[1].str());
    }
    
    // Extract content
    std::regex content_regex("\"content\"\\s*:\\s*\"([^\"]+)\"");
    std::string content;
    if (std::regex_search(payload, matches, content_regex)) {
        content = matches[1].str();
    }
    
    if (conversation_id <= 0 || content.empty()) {
        std::cerr << "Invalid chat message payload" << std::endl;
        return;
    }
    
    // Verify user is part of this conversation
    auto conversation_opt = conversation_repository_->getById(conversation_id);
    if (!conversation_opt.has_value()) {
        std::cerr << "Conversation not found: " << conversation_id << std::endl;
        return;
    }
    
    auto conversation = conversation_opt.value();
    if (conversation.user1_id != user_id && conversation.user2_id != user_id) {
        std::cerr << "User " << user_id << " not authorized for conversation " << conversation_id << std::endl;
        return;
    }
    
    // Create the message
    auto new_message_opt = message_repository_->createMessage(conversation_id, user_id, content);
    if (!new_message_opt.has_value()) {
        std::cerr << "Failed to create message" << std::endl;
        return;
    }
    
    auto new_message = new_message_opt.value();
    
    // Update conversation's last_message_at
    conversation_repository_->updateLastMessageTime(conversation_id);
    
    // Prepare message to send to both users
    std::ostringstream msg_json;
    msg_json << "{\"id\":" << new_message.id
             << ",\"conversation_id\":" << new_message.conversation_id
             << ",\"sender_id\":" << new_message.sender_id
             << ",\"content\":\"" << new_message.content << "\""
             << ",\"created_at\":\"" << new_message.created_at << "\"}";
    
    WebSocketMessage ws_message("chat:message", msg_json.str());
    
    // Send to both participants
    std::set<int> participants;
    participants.insert(conversation.user1_id);
    participants.insert(conversation.user2_id);
    websocket_server_->sendToUsers(participants, ws_message);
}

void AcademicSocialServer::handleTypingIndicator(int user_id, const WebSocketMessage& message) {
    // Parse payload to extract conversation_id
    std::string payload = message.payload;
    
    std::regex conv_regex("\"conversation_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;
    int conversation_id = 0;
    if (std::regex_search(payload, matches, conv_regex)) {
        conversation_id = std::stoi(matches[1].str());
    }
    
    if (conversation_id <= 0) {
        return;
    }
    
    // Verify user is part of this conversation
    auto conversation_opt = conversation_repository_->getById(conversation_id);
    if (!conversation_opt.has_value()) {
        return;
    }
    
    auto conversation = conversation_opt.value();
    if (conversation.user1_id != user_id && conversation.user2_id != user_id) {
        return;
    }
    
    // Determine the other user
    int other_user_id = (conversation.user1_id == user_id) 
        ? conversation.user2_id : conversation.user1_id;
    
    // Forward typing indicator to the other user
    std::ostringstream typing_json;
    typing_json << "{\"conversation_id\":" << conversation_id
                << ",\"user_id\":" << user_id << "}";
    
    WebSocketMessage ws_message("chat:typing", typing_json.str());
    websocket_server_->sendToUser(other_user_id, ws_message);
}

// Voice WebSocket handler implementations

void AcademicSocialServer::handleVoiceJoin(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract channel_id
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;
    int channel_id = 0;
    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }

    if (channel_id <= 0) {
        std::cerr << "Invalid voice:join payload - missing channel_id" << std::endl;
        return;
    }

    // Get user info
    auto user_opt = user_repository_->findById(user_id);
    if (!user_opt.has_value()) {
        std::cerr << "User not found: " << user_id << std::endl;
        return;
    }
    auto user = user_opt.value();

    // Add user to voice channel participants
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        voice_channel_participants_[channel_id].insert(user_id);
    }

    std::cout << "User " << user.getUsername() << " (id=" << user_id
              << ") joined voice channel " << channel_id << std::endl;

    // Notify all users in the channel about the new participant
    std::set<int> participants;
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it != voice_channel_participants_.end()) {
            participants = it->second;
        }
    }

    // Prepare join notification with user info
    std::string university_str = user.getUniversity().has_value() ? user.getUniversity().value() : "";
    std::string escaped_username = escapeJsonString(user.getUsername());
    std::string escaped_university = escapeJsonString(university_str);

    std::ostringstream join_json;
    join_json << "{\"channel_id\":" << channel_id
              << ",\"user_id\":" << user_id
              << ",\"username\":\"" << escaped_username << "\""
              << ",\"university\":\"" << escaped_university << "\"}";

    WebSocketMessage join_msg("voice:user-joined", join_json.str());
    websocket_server_->sendToUsers(participants, join_msg);

    // Send list of existing participants to the new user
    std::ostringstream participants_json;
    participants_json << "{\"channel_id\":" << channel_id << ",\"participants\":[";
    bool first = true;
    for (int participant_id : participants) {
        if (participant_id == user_id) continue; // Skip self

        auto participant_opt = user_repository_->findById(participant_id);
        if (participant_opt.has_value()) {
            auto participant = participant_opt.value();
            std::string participant_university = participant.getUniversity().has_value() ? participant.getUniversity().value() : "";
            std::string escaped_participant_username = escapeJsonString(participant.getUsername());
            std::string escaped_participant_university = escapeJsonString(participant_university);

            if (!first) participants_json << ",";
            participants_json << "{\"user_id\":" << participant_id
                             << ",\"username\":\"" << escaped_participant_username << "\""
                             << ",\"university\":\"" << escaped_participant_university << "\"}";
            first = false;
        }
    }
    participants_json << "]}";

    WebSocketMessage participants_msg("voice:participants", participants_json.str());
    websocket_server_->sendToUser(user_id, participants_msg);
}

void AcademicSocialServer::handleVoiceLeave(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract channel_id
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;
    int channel_id = 0;
    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }

    if (channel_id <= 0) {
        std::cerr << "Invalid voice:leave payload - missing channel_id" << std::endl;
        return;
    }

    // Remove user from channel and get remaining participants
    std::set<int> remaining_participants;
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it != voice_channel_participants_.end()) {
            it->second.erase(user_id);

            // Get remaining participants AFTER removal
            remaining_participants = it->second;

            // Clean up empty channels
            if (it->second.empty()) {
                voice_channel_participants_.erase(it);
            }
        }
    }

    std::cout << "User " << user_id << " left voice channel " << channel_id << std::endl;

    // Notify remaining users (excluding the leaving user) about the departure
    std::ostringstream leave_json;
    leave_json << "{\"channel_id\":" << channel_id
               << ",\"user_id\":" << user_id << "}";

    WebSocketMessage leave_msg("voice:user-left", leave_json.str());
    websocket_server_->sendToUsers(remaining_participants, leave_msg);
}

void AcademicSocialServer::handleVoiceOffer(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract target_user_id and channel_id
    std::regex target_regex("\"target_user_id\"\\s*:\\s*(\\d+)");
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;

    int target_user_id = 0;
    int channel_id = 0;

    if (std::regex_search(payload, matches, target_regex)) {
        target_user_id = std::stoi(matches[1].str());
    }
    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }

    if (target_user_id <= 0 || channel_id <= 0) {
        std::cerr << "Invalid voice:offer payload" << std::endl;
        return;
    }

    // Verify both users are in the same channel
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it == voice_channel_participants_.end() ||
            it->second.find(user_id) == it->second.end() ||
            it->second.find(target_user_id) == it->second.end()) {
            std::cerr << "Users not in same voice channel" << std::endl;
            return;
        }
    }

    // Forward the offer to the target user, including sender info
    WebSocketMessage offer_msg("voice:offer", payload);
    websocket_server_->sendToUser(target_user_id, offer_msg);
}

void AcademicSocialServer::handleVoiceAnswer(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract target_user_id and channel_id
    std::regex target_regex("\"target_user_id\"\\s*:\\s*(\\d+)");
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;

    int target_user_id = 0;
    int channel_id = 0;

    if (std::regex_search(payload, matches, target_regex)) {
        target_user_id = std::stoi(matches[1].str());
    }
    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }

    if (target_user_id <= 0 || channel_id <= 0) {
        std::cerr << "Invalid voice:answer payload" << std::endl;
        return;
    }

    // Verify both users are in the same channel
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it == voice_channel_participants_.end() ||
            it->second.find(user_id) == it->second.end() ||
            it->second.find(target_user_id) == it->second.end()) {
            std::cerr << "Users not in same voice channel" << std::endl;
            return;
        }
    }

    // Forward the answer to the target user
    WebSocketMessage answer_msg("voice:answer", payload);
    websocket_server_->sendToUser(target_user_id, answer_msg);
}

void AcademicSocialServer::handleVoiceIceCandidate(int user_id, const WebSocketMessage& message) {
    (void)user_id; // Unused - we just forward the ICE candidate
    std::string payload = message.payload;

    // Extract target_user_id and channel_id
    std::regex target_regex("\"target_user_id\"\\s*:\\s*(\\d+)");
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::smatch matches;

    int target_user_id = 0;
    int channel_id = 0;

    if (std::regex_search(payload, matches, target_regex)) {
        target_user_id = std::stoi(matches[1].str());
    }
    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }

    if (target_user_id <= 0 || channel_id <= 0) {
        std::cerr << "Invalid voice:ice-candidate payload" << std::endl;
        return;
    }

    // Forward ICE candidate to target user
    WebSocketMessage ice_msg("voice:ice-candidate", payload);
    websocket_server_->sendToUser(target_user_id, ice_msg);
}

void AcademicSocialServer::handleVoiceMute(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract channel_id and muted state
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::regex muted_regex("\"muted\"\\s*:\\s*(true|false)");
    std::smatch matches;

    int channel_id = 0;
    bool muted = false;

    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }
    if (std::regex_search(payload, matches, muted_regex)) {
        muted = (matches[1].str() == "true");
    }

    if (channel_id <= 0) {
        std::cerr << "Invalid voice:mute payload" << std::endl;
        return;
    }

    // Get participants in the channel
    std::set<int> participants;
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it != voice_channel_participants_.end()) {
            participants = it->second;
        }
    }

    // Broadcast mute status to all users in channel
    std::ostringstream mute_json;
    mute_json << "{\"channel_id\":" << channel_id
              << ",\"user_id\":" << user_id
              << ",\"muted\":" << (muted ? "true" : "false") << "}";

    WebSocketMessage mute_msg("voice:user-muted", mute_json.str());
    websocket_server_->sendToUsers(participants, mute_msg);
}

void AcademicSocialServer::handleVoiceVideoToggle(int user_id, const WebSocketMessage& message) {
    std::string payload = message.payload;

    // Extract channel_id and video_enabled state
    std::regex channel_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::regex video_regex("\"video_enabled\"\\s*:\\s*(true|false)");
    std::smatch matches;

    int channel_id = 0;
    bool video_enabled = false;

    if (std::regex_search(payload, matches, channel_regex)) {
        channel_id = std::stoi(matches[1].str());
    }
    if (std::regex_search(payload, matches, video_regex)) {
        video_enabled = (matches[1].str() == "true");
    }

    if (channel_id <= 0) {
        std::cerr << "Invalid voice:video-toggle payload" << std::endl;
        return;
    }

    // Get participants in the channel
    std::set<int> participants;
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        auto it = voice_channel_participants_.find(channel_id);
        if (it != voice_channel_participants_.end()) {
            participants = it->second;
        }
    }

    // Broadcast video toggle status to all users in channel
    std::ostringstream video_json;
    video_json << "{\"channel_id\":" << channel_id
               << ",\"user_id\":" << user_id
               << ",\"video_enabled\":" << (video_enabled ? "true" : "false") << "}";

    WebSocketMessage video_msg("voice:user-video-toggled", video_json.str());
    websocket_server_->sendToUsers(participants, video_msg);
}

void AcademicSocialServer::handleUserDisconnect(int user_id) {
    std::cout << "Cleaning up voice sessions for disconnected user: " << user_id << std::endl;

    // Find all channels the user was in
    std::vector<int> channels_to_notify;
    {
        std::lock_guard<std::mutex> lock(voice_channels_mutex_);
        for (auto& [channel_id, participants] : voice_channel_participants_) {
            if (participants.find(user_id) != participants.end()) {
                channels_to_notify.push_back(channel_id);
                participants.erase(user_id);

                // Clean up empty channels
                if (participants.empty()) {
                    voice_channel_participants_.erase(channel_id);
                }
            }
        }
    }

    // Notify other users in those channels that this user left
    for (int channel_id : channels_to_notify) {
        std::set<int> remaining_participants;
        {
            std::lock_guard<std::mutex> lock(voice_channels_mutex_);
            auto it = voice_channel_participants_.find(channel_id);
            if (it != voice_channel_participants_.end()) {
                remaining_participants = it->second;
            }
        }

        std::ostringstream leave_json;
        leave_json << "{\"channel_id\":" << channel_id
                   << ",\"user_id\":" << user_id << "}";

        WebSocketMessage leave_msg("voice:user-left", leave_json.str());
        websocket_server_->sendToUsers(remaining_participants, leave_msg);

        std::cout << "Notified channel " << channel_id << " that user " << user_id << " left" << std::endl;
    }

    // End all active voice sessions in the database
    if (voice_channel_repository_) {
        int sessions_ended = voice_channel_repository_->endAllUserSessions(user_id);
        std::cout << "Ended " << sessions_ended << " voice session(s) for user " << user_id << std::endl;
    }
}

void AcademicSocialServer::runVoiceChannelCleanup() {
    std::cout << "Voice channel cleanup task started" << std::endl;

    while (cleanup_running_) {
        // Sleep for 5 minutes before checking
        for (int i = 0; i < 300 && cleanup_running_; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (!cleanup_running_) break;

        // Find and delete empty inactive channels
        if (voice_channel_repository_) {
            std::vector<int> inactive_channels = voice_channel_repository_->findEmptyInactiveChannels(30);

            if (!inactive_channels.empty()) {
                std::cout << "Found " << inactive_channels.size()
                         << " voice channel(s) empty for more than 30 minutes" << std::endl;

                for (int channel_id : inactive_channels) {
                    if (voice_channel_repository_->deleteById(channel_id)) {
                        std::cout << "Closed empty voice channel: " << channel_id << std::endl;

                        // Clean up in-memory state if present
                        {
                            std::lock_guard<std::mutex> lock(voice_channels_mutex_);
                            voice_channel_participants_.erase(channel_id);
                        }
                    } else {
                        std::cerr << "Failed to close voice channel: " << channel_id << std::endl;
                    }
                }
            }
        }
    }

    std::cout << "Voice channel cleanup task stopped" << std::endl;
}

// Voice/Murmur handler implementations

HttpResponse AcademicSocialServer::handleCreateVoiceChannel(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id <= 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    std::string name = extractJsonField(request.body, "name");
    std::string channel_type = extractJsonField(request.body, "channel_type");
    
    if (name.empty()) {
        return createErrorResponse(400, "Channel name is required");
    }
    
    if (channel_type.empty()) {
        channel_type = "public";  // Default to public (Khave)
    }
    
    // Validate channel_type
    if (channel_type != "public" && channel_type != "group" && channel_type != "private") {
        return createErrorResponse(400, "Invalid channel_type. Must be 'public', 'group', or 'private'");
    }
    
    // Extract optional group_id and organization_id
    int group_id = 0;
    int organization_id = 0;
    
    std::string group_id_str = extractJsonField(request.body, "group_id");
    if (!group_id_str.empty()) {
        group_id = std::stoi(group_id_str);
    }
    
    std::string org_id_str = extractJsonField(request.body, "organization_id");
    if (!org_id_str.empty()) {
        organization_id = std::stoi(org_id_str);
    }
    
    // Create channel using VoiceService
    VoiceChannel channel = voice_service_->create_channel(name, channel_type, group_id, organization_id);
    
    if (channel.id <= 0) {
        return createErrorResponse(500, "Failed to create voice channel");
    }
    
    // Save to database
    auto saved_channel = voice_channel_repository_->create(channel);
    if (!saved_channel.has_value()) {
        return createErrorResponse(500, "Failed to save voice channel to database");
    }

    // Construct JSON response with active_users field (always 0 for newly created channel)
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << saved_channel.value().id << ",";
    oss << "\"name\":\"" << saved_channel.value().name << "\",";
    oss << "\"channel_type\":\"" << saved_channel.value().channel_type << "\",";
    oss << "\"active_users\":0,";

    // Convert time_t to ISO 8601 string
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ",
                  std::gmtime(&saved_channel.value().created_at));
    oss << "\"created_at\":\"" << time_buf << "\"";
    oss << "}";

    return createJsonResponse(201, oss.str());
}

HttpResponse AcademicSocialServer::handleGetVoiceChannels(const HttpRequest& request) {
    int limit = 50;
    int offset = 0;
    std::string channel_type;
    
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::regex type_regex("[?&]channel_type=([^&]+)");
    std::smatch match;
    
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }
    
    if (std::regex_search(request.path, match, type_regex)) {
        channel_type = match[1].str();
    }
    
    std::vector<VoiceChannel> channels;
    if (!channel_type.empty()) {
        channels = voice_channel_repository_->findByType(channel_type, limit, offset);
    } else {
        channels = voice_channel_repository_->findAll(limit, offset);
    }
    
    // Get active user counts for each channel
    std::ostringstream oss;
    oss << "{\"channels\":[";
    
    for (size_t i = 0; i < channels.size(); ++i) {
        oss << "{";
        oss << "\"id\":" << channels[i].id << ",";
        oss << "\"name\":\"" << channels[i].name << "\",";
        oss << "\"channel_type\":\"" << channels[i].channel_type << "\",";
        oss << "\"active_users\":" << voice_channel_repository_->getActiveUserCount(channels[i].id) << ",";
        oss << "\"created_at\":\"" << channels[i].created_at << "\"";
        oss << "}";
        
        if (i < channels.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "],\"count\":" << channels.size() << "}";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetVoiceChannel(const HttpRequest& request) {
    int channel_id = extractIdFromPath(request.path, "/api/voice/channels/");
    
    if (channel_id <= 0) {
        return createErrorResponse(400, "Invalid channel ID");
    }
    
    auto channel_opt = voice_channel_repository_->findById(channel_id);
    if (!channel_opt.has_value()) {
        return createErrorResponse(404, "Voice channel not found");
    }
    
    auto channel = channel_opt.value();
    int active_users = voice_channel_repository_->getActiveUserCount(channel_id);
    
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << channel.id << ",";
    oss << "\"name\":\"" << channel.name << "\",";
    oss << "\"channel_type\":\"" << channel.channel_type << "\",";
    oss << "\"active_users\":" << active_users << ",";
    oss << "\"created_at\":\"" << channel.created_at << "\"";
    oss << "}";
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleJoinVoiceChannel(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id <= 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int channel_id = extractIdFromPath(request.path, "/api/voice/channels/");
    
    if (channel_id <= 0) {
        return createErrorResponse(400, "Invalid channel ID");
    }
    
    // Verify channel exists
    auto channel_opt = voice_channel_repository_->findById(channel_id);
    if (!channel_opt.has_value()) {
        return createErrorResponse(404, "Voice channel not found");
    }
    
    // Check if user already has an active session and clean it up if exists
    // This allows users to re-enter after page refresh or navigation
    int existing_session = voice_channel_repository_->getUserActiveSession(user_id, channel_id);
    if (existing_session > 0) {
        // End the existing session to allow re-entry
        voice_channel_repository_->endSession(existing_session);
    }
    
    // Generate connection token
    VoiceConnectionToken token = voice_service_->generate_connection_token(user_id, channel_id);
    
    // Create session in database
    int session_id = voice_channel_repository_->createSession(channel_id, user_id);
    
    if (session_id <= 0) {
        return createErrorResponse(500, "Failed to create voice session");
    }
    
    std::ostringstream oss;
    oss << "{";
    oss << "\"session_id\":" << session_id << ",";
    oss << "\"channel_id\":" << channel_id << ",";
    oss << token.to_json().substr(1); // Remove opening { from token.to_json()
    
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleLeaveVoiceChannel(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id <= 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int channel_id = extractIdFromPath(request.path, "/api/voice/channels/");
    
    if (channel_id <= 0) {
        return createErrorResponse(400, "Invalid channel ID");
    }
    
    // Get user's active session
    int session_id = voice_channel_repository_->getUserActiveSession(user_id, channel_id);
    
    if (session_id <= 0) {
        return createErrorResponse(404, "No active session found");
    }
    
    // End the session
    if (!voice_channel_repository_->endSession(session_id)) {
        return createErrorResponse(500, "Failed to end voice session");
    }
    
    return createJsonResponse(200, "{\"message\":\"Left voice channel successfully\"}");
}

HttpResponse AcademicSocialServer::handleDeleteVoiceChannel(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id <= 0) {
        return createErrorResponse(401, "Unauthorized");
    }
    
    int channel_id = extractIdFromPath(request.path, "/api/voice/channels/");
    
    if (channel_id <= 0) {
        return createErrorResponse(400, "Invalid channel ID");
    }
    
    // Verify channel exists
    auto channel_opt = voice_channel_repository_->findById(channel_id);
    if (!channel_opt.has_value()) {
        return createErrorResponse(404, "Voice channel not found");
    }
    
    // Delete the channel using voice service
    if (!voice_service_->delete_channel(channel_id)) {
        return createErrorResponse(500, "Failed to delete voice channel from service");
    }
    
    // Delete from database
    if (!voice_channel_repository_->deleteById(channel_id)) {
        return createErrorResponse(500, "Failed to delete voice channel from database");
    }
    
    return createJsonResponse(200, "{\"message\":\"Voice channel deleted successfully\"}");
}

// ==================== Hashtag Handlers ====================

HttpResponse AcademicSocialServer::handleGetTrendingHashtags(const HttpRequest& request) {
    // Parse limit parameter
    int limit = 10;
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::smatch match;
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }

    auto hashtags = hashtag_repository_->findTrending(limit);

    std::ostringstream oss;
    oss << "{\"hashtags\":[";
    for (size_t i = 0; i < hashtags.size(); ++i) {
        oss << hashtags[i].toJson();
        if (i < hashtags.size() - 1) oss << ",";
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleSearchHashtags(const HttpRequest& request) {
    // Parse query and limit parameters
    std::string query = "";
    int limit = 20;

    std::regex query_regex("[?&]q=([^&]+)");
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::smatch match;

    if (std::regex_search(request.path, match, query_regex)) {
        query = match[1].str();
    }
    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }

    if (query.empty()) {
        return createErrorResponse(400, "Query parameter 'q' is required");
    }

    auto hashtags = hashtag_repository_->searchTags(query, limit);

    std::ostringstream oss;
    oss << "{\"hashtags\":[";
    for (size_t i = 0; i < hashtags.size(); ++i) {
        oss << hashtags[i].toJson();
        if (i < hashtags.size() - 1) oss << ",";
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetPostsByHashtag(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    // Extract hashtag from path (/api/hashtags/{tag}/posts)
    std::string tag;
    size_t pos = request.path.find("/api/hashtags/");
    if (pos != std::string::npos) {
        size_t start = pos + 14; // length of "/api/hashtags/"
        size_t end = request.path.find("/", start);
        if (end != std::string::npos) {
            tag = request.path.substr(start, end - start);
        }
    }

    if (tag.empty()) {
        return createErrorResponse(400, "Invalid hashtag");
    }

    // Find hashtag
    auto hashtag = hashtag_repository_->findByTag(tag);
    if (!hashtag.has_value() || !hashtag->getId().has_value()) {
        return createJsonResponse(200, "{\"posts\":[]}");
    }

    // Get posts with this hashtag
    std::ostringstream oss;
    oss << "{\"tag\":\"" << tag << "\",\"posts\":[";

    // TODO: Implement getting posts by hashtag ID
    // For now return empty array
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

// ==================== Announcement Handlers ====================

HttpResponse AcademicSocialServer::handleCreateAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    // Extract group_id from path
    int group_id = -1;
    size_t pos = request.path.find("/api/groups/");
    if (pos != std::string::npos) {
        size_t start = pos + 12; // length of "/api/groups/"
        size_t end = request.path.find("/", start);
        if (end != std::string::npos) {
            std::string id_str = request.path.substr(start, end - start);
            group_id = std::stoi(id_str);
        }
    }

    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }

    // Check if user is group admin or moderator
    std::string member_role = group_repository_->getMemberRole(group_id, user_id);
    if (member_role.empty() || (member_role != "admin" && member_role != "moderator")) {
        return createErrorResponse(403, "Only group admins and moderators can create announcements");
    }

    std::string title = extractJsonField(request.body, "title");
    std::string content = extractJsonField(request.body, "content");
    std::string is_pinned_str = extractJsonField(request.body, "is_pinned");

    if (title.empty() || content.empty()) {
        return createErrorResponse(400, "title and content are required");
    }

    Announcement announcement(group_id, user_id, title, content);
    if (!is_pinned_str.empty() && is_pinned_str == "true") {
        announcement.setPinned(true);
    }

    auto created = announcement_repository_->create(announcement);
    if (created.has_value()) {
        // Populate author information
        auto author = user_repository_->findById(user_id);
        if (author.has_value()) {
            created->setAuthorUsername(author->getUsername());
            if (author->getName().has_value()) {
                created->setAuthorName(author->getName().value());
            }
        }

        // TODO: Create notifications for all group members
        return createJsonResponse(201, created->toJson());
    }

    return createErrorResponse(500, "Failed to create announcement");
}

HttpResponse AcademicSocialServer::handleGetAnnouncements(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    // Extract group_id from path
    int group_id = -1;
    size_t pos = request.path.find("/api/groups/");
    if (pos != std::string::npos) {
        size_t start = pos + 12; // length of "/api/groups/"
        size_t end = request.path.find("/", start);
        if (end != std::string::npos) {
            std::string id_str = request.path.substr(start, end - start);
            group_id = std::stoi(id_str);
        }
    }

    if (group_id < 0) {
        return createErrorResponse(400, "Invalid group ID");
    }

    // Check if user is a member of the group
    std::string member_role = group_repository_->getMemberRole(group_id, user_id);
    if (member_role.empty()) {
        return createErrorResponse(403, "You must be a member of this group");
    }

    // Parse pagination parameters
    int limit = 50;
    int offset = 0;
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;

    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }

    auto announcements = announcement_repository_->findByGroupId(group_id, false, limit, offset);

    std::ostringstream oss;
    oss << "{\"announcements\":[";
    for (size_t i = 0; i < announcements.size(); ++i) {
        oss << announcements[i].toJson();
        if (i < announcements.size() - 1) oss << ",";
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleGetAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int announcement_id = extractIdFromPath(request.path, "/api/announcements/");
    if (announcement_id < 0) {
        return createErrorResponse(400, "Invalid announcement ID");
    }

    auto announcement = announcement_repository_->findById(announcement_id);
    if (!announcement.has_value()) {
        return createErrorResponse(404, "Announcement not found");
    }

    // Check if user is a member of the group
    std::string member_role = group_repository_->getMemberRole(announcement->getGroupId(), user_id);
    if (member_role.empty()) {
        return createErrorResponse(403, "You must be a member of this group");
    }

    return createJsonResponse(200, announcement->toJson());
}

HttpResponse AcademicSocialServer::handleUpdateAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int announcement_id = extractIdFromPath(request.path, "/api/announcements/");
    if (announcement_id < 0) {
        return createErrorResponse(400, "Invalid announcement ID");
    }

    // Check if user can manage this announcement
    if (!announcement_repository_->canUserManage(announcement_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to update this announcement");
    }

    auto announcement = announcement_repository_->findById(announcement_id);
    if (!announcement.has_value()) {
        return createErrorResponse(404, "Announcement not found");
    }

    std::string title = extractJsonField(request.body, "title");
    std::string content = extractJsonField(request.body, "content");

    if (!title.empty()) {
        announcement->setTitle(title);
    }
    if (!content.empty()) {
        announcement->setContent(content);
    }

    if (announcement_repository_->update(announcement.value())) {
        return createJsonResponse(200, announcement->toJson());
    }

    return createErrorResponse(500, "Failed to update announcement");
}

HttpResponse AcademicSocialServer::handleDeleteAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int announcement_id = extractIdFromPath(request.path, "/api/announcements/");
    if (announcement_id < 0) {
        return createErrorResponse(400, "Invalid announcement ID");
    }

    // Check if user can manage this announcement
    if (!announcement_repository_->canUserManage(announcement_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to delete this announcement");
    }

    if (announcement_repository_->deleteById(announcement_id)) {
        return HttpResponse(204, "text/plain", "");
    }

    return createErrorResponse(500, "Failed to delete announcement");
}

HttpResponse AcademicSocialServer::handlePinAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int announcement_id = extractIdFromPath(request.path, "/api/announcements/");
    if (announcement_id < 0) {
        return createErrorResponse(400, "Invalid announcement ID");
    }

    // Check if user can manage this announcement
    if (!announcement_repository_->canUserManage(announcement_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to pin this announcement");
    }

    if (announcement_repository_->pin(announcement_id)) {
        auto announcement = announcement_repository_->findById(announcement_id);
        if (announcement.has_value()) {
            return createJsonResponse(200, announcement->toJson());
        }
    }

    return createErrorResponse(500, "Failed to pin announcement");
}

HttpResponse AcademicSocialServer::handleUnpinAnnouncement(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int announcement_id = extractIdFromPath(request.path, "/api/announcements/");
    if (announcement_id < 0) {
        return createErrorResponse(400, "Invalid announcement ID");
    }

    // Check if user can manage this announcement
    if (!announcement_repository_->canUserManage(announcement_id, user_id)) {
        return createErrorResponse(403, "You don't have permission to unpin this announcement");
    }

    if (announcement_repository_->unpin(announcement_id)) {
        auto announcement = announcement_repository_->findById(announcement_id);
        if (announcement.has_value()) {
            return createJsonResponse(200, announcement->toJson());
        }
    }

    return createErrorResponse(500, "Failed to unpin announcement");
}

// ==================== Mention Handlers ====================

HttpResponse AcademicSocialServer::handleGetUserMentions(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    // Extract user_id from path or use authenticated user
    int target_user_id = user_id;
    size_t pos = request.path.find("/api/users/");
    if (pos != std::string::npos) {
        size_t start = pos + 11; // length of "/api/users/"
        size_t end = request.path.find("/", start);
        if (end != std::string::npos) {
            std::string id_str = request.path.substr(start, end - start);
            try {
                target_user_id = std::stoi(id_str);
            } catch (...) {
                // Keep using authenticated user_id if parsing fails
            }
        }
    }

    // Users can only see their own mentions
    if (target_user_id != user_id) {
        return createErrorResponse(403, "You can only view your own mentions");
    }

    // Parse pagination parameters
    int limit = 50;
    int offset = 0;
    std::regex limit_regex("[?&]limit=(\\d+)");
    std::regex offset_regex("[?&]offset=(\\d+)");
    std::smatch match;

    if (std::regex_search(request.path, match, limit_regex)) {
        limit = std::stoi(match[1].str());
    }
    if (std::regex_search(request.path, match, offset_regex)) {
        offset = std::stoi(match[1].str());
    }

    auto post_ids = mention_repository_->findPostIdsByUserId(user_id, limit, offset);

    // Fetch the actual posts
    std::ostringstream oss;
    oss << "{\"mentions\":[";
    for (size_t i = 0; i < post_ids.size(); ++i) {
        auto post = post_repository_->findById(post_ids[i]);
        if (post.has_value()) {
            // Populate author information
            auto author = user_repository_->findById(post->getAuthorId());
            if (author.has_value()) {
                post->setAuthorUsername(author->getUsername());
                if (author->getName().has_value()) {
                    post->setAuthorName(author->getName().value());
                }
                if (author->getAvatarUrl().has_value()) {
                    post->setAuthorAvatarUrl(author->getAvatarUrl().value());
                }
            }
            oss << post->toJson();
            if (i < post_ids.size() - 1) oss << ",";
        }
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

// -------------------- Study Buddy Matching Handlers --------------------

HttpResponse AcademicSocialServer::handleGetStudyPreferences(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    auto prefs = study_preferences_repository_->findByUserId(user_id);
    if (!prefs) {
        return createJsonResponse(200, "{\"has_preferences\": false}");
    }

    return createJsonResponse(200, prefs->toJson().dump());
}

HttpResponse AcademicSocialServer::handleSetStudyPreferences(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    try {
        json j = json::parse(request.body);

        StudyPreferences prefs;
        prefs.user_id = user_id;
        prefs.learning_style = StudyPreferences::stringToLearningStyle(j.value("learning_style", "mixed"));
        prefs.study_environment = StudyPreferences::stringToStudyEnvironment(j.value("study_environment", "flexible"));
        prefs.study_time_preference = StudyPreferences::stringToStudyTimePreference(j.value("study_time_preference", "flexible"));

        if (j.contains("courses") && j["courses"].is_array()) {
            for (const auto& course : j["courses"]) {
                prefs.courses.push_back(course.get<std::string>());
            }
        }

        if (j.contains("topics_of_interest") && j["topics_of_interest"].is_array()) {
            for (const auto& topic : j["topics_of_interest"]) {
                prefs.topics_of_interest.push_back(topic.get<std::string>());
            }
        }

        if (j.contains("available_days") && j["available_days"].is_array()) {
            for (const auto& day : j["available_days"]) {
                prefs.available_days.push_back(day.get<std::string>());
            }
        }

        prefs.academic_goals = j.value("academic_goals", "");
        prefs.available_hours_per_week = j.value("available_hours_per_week", 5);
        prefs.preferred_group_size = j.value("preferred_group_size", 2);
        prefs.same_university_only = j.value("same_university_only", true);
        prefs.same_department_only = j.value("same_department_only", false);
        prefs.same_year_only = j.value("same_year_only", false);
        prefs.is_active = j.value("is_active", true);

        auto saved = study_preferences_repository_->upsert(prefs);
        if (!saved) {
            return createErrorResponse(500, "Failed to save preferences");
        }

        return createJsonResponse(200, saved->toJson().dump());
    } catch (const std::exception& e) {
        return createErrorResponse(400, std::string("Invalid request: ") + e.what());
    }
}

HttpResponse AcademicSocialServer::handleGetStudyBuddyMatches(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    auto matches = study_buddy_matching_service_->getRecommendations(user_id, 20);

    std::ostringstream oss;
    oss << "{\"matches\":[";
    for (size_t i = 0; i < matches.size(); ++i) {
        if (i > 0) oss << ",";

        json matchJson = matches[i].toJson();

        // Add matched user details
        auto matchedUser = user_repository_->findById(matches[i].matched_user_id);
        if (matchedUser) {
            json userJson;
            userJson["id"] = matchedUser->getId().value_or(0);
            userJson["username"] = matchedUser->getUsername();
            userJson["name"] = matchedUser->getName().value_or("");
            userJson["university"] = matchedUser->getUniversity().value_or("");
            userJson["department"] = matchedUser->getDepartment().value_or("");
            userJson["enrollment_year"] = matchedUser->getEnrollmentYear().value_or(0);
            matchJson["matched_user"] = userJson;
        }

        oss << matchJson.dump();
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleRefreshStudyBuddyMatches(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int count = study_buddy_matching_service_->refreshMatches(user_id);

    std::ostringstream oss;
    oss << "{\"matches_generated\":" << count << "}";
    return createJsonResponse(200, oss.str());
}

HttpResponse AcademicSocialServer::handleAcceptStudyBuddyMatch(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int match_id = extractIdFromPath(request.path, "/api/study-buddies/matches/");
    if (match_id < 0) {
        return createErrorResponse(400, "Invalid match ID");
    }

    if (!study_buddy_match_repository_->updateStatus(match_id, MatchStatus::ACCEPTED)) {
        return createErrorResponse(500, "Failed to accept match");
    }

    // Create a study buddy connection
    auto match = study_buddy_match_repository_->findById(match_id);
    if (match) {
        StudyBuddyConnection conn;
        conn.user_id = user_id;
        conn.buddy_id = match->matched_user_id;
        conn.connection_strength = static_cast<int>(match->compatibility_score);
        study_buddy_connection_repository_->create(conn);
    }

    return createJsonResponse(200, "{\"status\":\"accepted\"}");
}

HttpResponse AcademicSocialServer::handleDeclineStudyBuddyMatch(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    int match_id = extractIdFromPath(request.path, "/api/study-buddies/matches/");
    if (match_id < 0) {
        return createErrorResponse(400, "Invalid match ID");
    }

    if (!study_buddy_match_repository_->updateStatus(match_id, MatchStatus::DECLINED)) {
        return createErrorResponse(500, "Failed to decline match");
    }

    return createJsonResponse(200, "{\"status\":\"declined\"}");
}

HttpResponse AcademicSocialServer::handleGetStudyBuddyConnections(const HttpRequest& request) {
    int user_id = getUserIdFromAuth(request);
    if (user_id < 0) {
        return createErrorResponse(401, "Unauthorized");
    }

    auto connections = study_buddy_connection_repository_->findByUserId(user_id);

    std::ostringstream oss;
    oss << "{\"connections\":[";
    for (size_t i = 0; i < connections.size(); ++i) {
        if (i > 0) oss << ",";

        json connJson = connections[i].toJson();

        // Add buddy user details
        auto buddy = user_repository_->findById(connections[i].buddy_id);
        if (buddy) {
            json buddyJson;
            buddyJson["id"] = buddy->getId().value_or(0);
            buddyJson["username"] = buddy->getUsername();
            buddyJson["name"] = buddy->getName().value_or("");
            buddyJson["university"] = buddy->getUniversity().value_or("");
            buddyJson["department"] = buddy->getDepartment().value_or("");
            connJson["buddy"] = buddyJson;
        }

        oss << connJson.dump();
    }
    oss << "]}";

    return createJsonResponse(200, oss.str());
}

} // namespace server
} // namespace sohbet
