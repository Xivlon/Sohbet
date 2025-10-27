#include "server/server.h"
#include "models/user.h"
#include "models/media.h"
#include "models/group.h"
#include "models/organization.h"
#include "security/jwt.h"
#include "utils/hash.h"
#include "utils/multipart_parser.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

namespace sohbet {
namespace server {

AcademicSocialServer::AcademicSocialServer(int port, const std::string& db_path)
    : port_(port), db_path_(db_path), running_(false), server_socket_(-1) {
}

bool AcademicSocialServer::initialize() {
    database_ = std::make_shared<db::Database>(db_path_);
    if (!database_->isOpen()) {
        std::cerr << "Failed to open database: " << db_path_ << std::endl;
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
    storage_service_ = std::make_shared<services::StorageService>("uploads/");
    
    // Initialize WebSocket server
    websocket_server_ = std::make_shared<WebSocketServer>(8081);
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

    // Ensure demo user exists for demo/testing purposes
    ensureDemoUserExists();

    std::cout << "Server initialized successfully" << std::endl;
    return true;
}

bool AcademicSocialServer::start() {
    std::cout << "Academic Social Server starting on port " << port_ << std::endl;
    std::cout << "Database: " << db_path_ << std::endl;
    std::cout << "Version: 0.2.0-academic" << std::endl;
    
    if (!initializeSocket()) {
        std::cerr << "Failed to initialize server socket" << std::endl;
        return false;
    }
    
    // Start WebSocket server
    if (!websocket_server_->start()) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        return false;
    }
    
    running_ = true;
    std::cout << "🌐 HTTP Server listening on http://localhost:" << port_ << std::endl;
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
                    if (bytes_read <= 0) break;
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
    
    // Format and send response
    std::string http_response = formatHttpResponse(response);
    send(client_socket, http_response.c_str(), http_response.length(), 0);
    
    close(client_socket);
}

HttpRequest AcademicSocialServer::parseHttpRequest(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line (GET /path HTTP/1.1)
    std::getline(stream, line);
    std::istringstream request_line(line);
    std::string method, path, version;
    request_line >> method >> path >> version;
    
    HttpRequest request(method, path, "");
    
    // Parse headers
    bool headers_done = false;
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) {
            headers_done = true;
            break;
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
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
            
            request.headers[header_name] = header_value;
        }
    }
    
    // Read body if present
    if (headers_done) {
        std::string remaining;
        while (std::getline(stream, line)) {
            request.body += line + "\n";
        }
    }
    
    return request;
}

std::string AcademicSocialServer::formatHttpResponse(const HttpResponse& response) {
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
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
}

// -------------------- Request Handlers --------------------

HttpResponse AcademicSocialServer::handleRequest(const HttpRequest& request) {
    std::string base_path = request.path;
    size_t query_pos = base_path.find('?');
    if (query_pos != std::string::npos) {
        base_path = base_path.substr(0, query_pos);
    }
    
    if (request.method == "OPTIONS") {
        return HttpResponse(204, "text/plain", "");
    } else if (request.method == "GET" && base_path == "/api/status") {
        return handleStatus(request);
    } else if (request.method == "GET" && base_path == "/api/users") {
        return handleGetUsers(request);
    } else if (request.method == "GET" && base_path == "/api/users/demo") {
        return handleUsersDemo(request);
    } else if (request.method == "POST" && base_path == "/api/users") {
        return handleCreateUser(request);
    } else if (request.method == "POST" && base_path == "/api/login") {
        return handleLogin(request);
    } else if (request.method == "PUT" && base_path.find("/api/users/") == 0) {
        return handleUpdateUser(request);
    } else if (request.method == "POST" && base_path == "/api/media/upload") {
        return handleUploadMedia(request);
    } else if (request.method == "GET" && base_path.find("/api/media/file/") == 0) {
        return handleGetMediaFile(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/media") != std::string::npos) {
        return handleGetUserMedia(request);
    }
    // Friendship routes
    else if (request.method == "POST" && base_path == "/api/friendships") {
        return handleCreateFriendship(request);
    } else if (request.method == "GET" && base_path == "/api/friendships") {
        return handleGetFriendships(request);
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/friends") != std::string::npos) {
        return handleGetFriends(request);
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
    } else if (request.method == "GET" && base_path.find("/api/users/") == 0 && base_path.find("/posts") != std::string::npos) {
        return handleGetUserPosts(request);
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
    } else {
        return handleNotFound(request);
    }
}

HttpResponse AcademicSocialServer::handleStatus(const HttpRequest& request) {
    (void)request;
    std::string response = R"({"status":"ok","version":"0.2.0-academic","features":["user_registration","sqlite_persistence","bcrypt_hashing"]})";
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
    return createJsonResponse(200, demo.toJson());
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
        std::string token = security::generate_jwt_token(username, user.getId().value(), user_role);

        std::ostringstream oss;
        oss << "{ \"token\":\"" << token << "\", \"user\":" << user.toJson() << " }";
        return createJsonResponse(200, oss.str());
    } catch (...) {
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
    std::regex str_regex("\"" + field + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, str_regex)) return match[1].str();

    std::regex num_regex("\"" + field + "\"\\s*:\\s*(\\d+)");
    if (std::regex_search(json, match, num_regex)) return match[1].str();

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
    // Check if demo user already exists
    auto existing_user = user_repository_->findByUsername("demo_student");
    if (existing_user.has_value()) {
        std::cout << "Demo user already exists" << std::endl;
        return;
    }

    // Create demo user
    User demo_user("demo_student", "demo@example.edu");
    demo_user.setUniversity("Demo University");
    demo_user.setDepartment("Computer Science");
    demo_user.setEnrollmentYear(2023);
    demo_user.setPrimaryLanguage("Turkish");

    auto created_user = user_repository_->create(demo_user, "demo123");
    if (created_user.has_value()) {
        std::cout << "Demo user created successfully (ID: " << created_user->getId().value() << ")" << std::endl;
    } else {
        std::cerr << "Warning: Failed to create demo user" << std::endl;
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

int AcademicSocialServer::getUserIdFromAuth(const HttpRequest& request) {
    // Extract user ID from Authorization header (JWT token)
    auto it = request.headers.find("Authorization");
    if (it != request.headers.end()) {
        std::string auth_header = it->second;
        if (auth_header.find("Bearer ") == 0) {
            std::string token = auth_header.substr(7);
            // TODO: Properly decode JWT and extract user ID
        }
    }
    
    // Fallback: check for X-User-ID header (for testing)
    it = request.headers.find("X-User-ID");
    if (it != request.headers.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return -1;
        }
    }
    
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
    
    if (friendship->getRequesterId() != user_id && friendship->getAddresseeId() != user_id) {
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
    oss << "[";
    for (size_t i = 0; i < posts.size(); ++i) {
        oss << posts[i].toJson();
        if (i < posts.size() - 1) oss << ",";
    }
    oss << "]";
    
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
    
    if (post->getAuthorId() != user_id) {
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
    
    if (post->getAuthorId() != user_id) {
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
    
    if (comment->getAuthorId() != user_id) {
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
    
    if (comment->getAuthorId() != user_id) {
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
    oss << "[";
    for (size_t i = 0; i < groups.size(); ++i) {
        if (i > 0) oss << ",";
        oss << groups[i].toJson();
    }
    oss << "]";
    
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
    
    std::vector<Organization> orgs = organization_repository_->findAll();
    
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < orgs.size(); ++i) {
        if (i > 0) oss << ",";
        oss << orgs[i].toJson();
    }
    oss << "]";
    
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

} // namespace server
} // namespace sohbet