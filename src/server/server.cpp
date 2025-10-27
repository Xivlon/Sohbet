#include "server/server.h"
#include "models/user.h"
#include "models/media.h"
#include "security/jwt.h"
#include "utils/hash.h"
#include "utils/multipart_parser.h"
#include <iostream>
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
    storage_service_ = std::make_shared<services::StorageService>("uploads/");

    if (!user_repository_->migrate()) {
        std::cerr << "Failed to run database migrations" << std::endl;
        return false;
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
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    
    // Read request
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\0';
    std::string raw_request(buffer);
    
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

} // namespace server
} // namespace sohbet