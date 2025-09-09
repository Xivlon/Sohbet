#include "server/server.h"
#include "models/user.h"
#include "security/jwt.h"
#include "utils/hash.h"
#include <iostream>
#include <regex>

namespace sohbet {

AcademicSocialServer::AcademicSocialServer(const std::string& db_path) {
    try {
        db_ = std::make_unique<Database>(db_path);
        user_repository_ = std::make_unique<UserRepository>(*db_);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize database: " << e.what() << std::endl;
    }
}

bool AcademicSocialServer::initialize() {
    if (!db_ || !user_repository_) {
        return false;
    }
    
    // Run migrations
    return user_repository_->migrate();
}

void AcademicSocialServer::run(int port) {
    std::cout << "Academic Social Server running on port " << port << std::endl;
    std::cout << "Version: 0.2.0-academic" << std::endl;
    std::cout << "Database initialized and ready for connections." << std::endl;
    
    // Simplified server - in real implementation this would be a proper HTTP server
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /api/status" << std::endl;
    std::cout << "  GET  /api/users/demo" << std::endl;
    std::cout << "  POST /api/users" << std::endl;
    std::cout << "  POST /api/login" << std::endl;
    std::cout << "  GET  /api/users/:username" << std::endl;
}

std::string AcademicSocialServer::handleStatusRequest() {
    return R"({"status":"ok","version":"0.2.0-academic","features":["user_registration","sqlite_persistence","placeholder_hashing"]})";
}

std::string AcademicSocialServer::handleUsersDemoRequest() {
    User demo_user("demo_student", "demo@example.edu");
    demo_user.setId(999);
    demo_user.setUniversity("Demo University");
    demo_user.setDepartment("Computer Science");
    demo_user.setEnrollmentYear(2023);
    demo_user.setPrimaryLanguage("Turkish");
    
    return demo_user.toJson();
}

std::string AcademicSocialServer::handleUserRegistration(const std::string& request_body) {
    try {
        // Parse user from JSON
        User user = User::fromJson(request_body);
        
        // Extract password from JSON (basic parsing)
        std::regex password_regex("\"password\"\\s*:\\s*\"([^\"]+)\"");
        std::smatch password_match;
        if (!std::regex_search(request_body, password_match, password_regex)) {
            return jsonError("Password is required", 400);
        }
        std::string password = password_match[1].str();
        
        // Validation
        if (!validateUsername(user.getUsername())) {
            return jsonError("Invalid username format. Must be 3-32 characters, alphanumeric and underscore only.", 400);
        }
        
        if (!validateEmail(user.getEmail())) {
            return jsonError("Invalid email format", 400);
        }
        
        if (password.length() < 8) {
            return jsonError("Password must be at least 8 characters long", 400);
        }
        
        // Check uniqueness
        if (user_repository_->usernameExists(user.getUsername())) {
            return jsonError("Username already exists", 409);
        }
        
        if (user_repository_->emailExists(user.getEmail())) {
            return jsonError("Email already exists", 409);
        }
        
        // Create user
        auto created_user = user_repository_->create(user, password);
        if (!created_user.has_value()) {
            return jsonError("Failed to create user", 500);
        }
        
        return created_user.value().toJson();
        
    } catch (const std::exception& e) {
        return jsonError("Invalid request format", 400);
    }
}

std::string AcademicSocialServer::handleLogin(const std::string& request_body) {
    try {
        // Parse username and password from JSON
        std::regex username_regex("\"username\"\\s*:\\s*\"([^\"]+)\"");
        std::regex password_regex("\"password\"\\s*:\\s*\"([^\"]+)\"");
        std::smatch username_match, password_match;
        
        if (!std::regex_search(request_body, username_match, username_regex) ||
            !std::regex_search(request_body, password_match, password_regex)) {
            return jsonError("Username and password are required", 400);
        }
        
        std::string username = username_match[1].str();
        std::string password = password_match[1].str();
        
        // Find user by username
        auto user_opt = user_repository_->findByUsername(username);
        if (!user_opt.has_value()) {
            return jsonError("Invalid username or password", 401);
        }
        
        User user = user_opt.value();
        
        // Verify password using bcrypt
        if (!utils::verify_password(password, user.getPasswordHash())) {
            return jsonError("Invalid username or password", 401);
        }
        
        // Generate JWT token
        std::string token = security::generate_jwt_token(username, user.getId().value());
        
        // Return success response with token
        std::ostringstream response;
        response << "{";
        response << "\"token\":\"" << token << "\",";
        response << "\"user\":" << user.toJson();
        response << "}";
        
        return response.str();
        
    } catch (const std::exception& e) {
        return jsonError("Invalid request format", 400);
    }
}

std::string AcademicSocialServer::handleGetUser(const std::string& username) {
    try {
        // Find user by username
        auto user_opt = user_repository_->findByUsername(username);
        if (!user_opt.has_value()) {
            return jsonError("User not found", 404);
        }
        
        User user = user_opt.value();
        
        // Return public user profile (password hash is not included in toJson())
        return user.toJson();
        
    } catch (const std::exception& e) {
        return jsonError("Internal server error", 500);
    }
}

std::string AcademicSocialServer::jsonError(const std::string& message, int code) {
    return R"({"error":")" + message + R"(","code":)" + std::to_string(code) + "}";
}

bool AcademicSocialServer::validateUsername(const std::string& username) {
    std::regex username_pattern("^[A-Za-z0-9_]{3,32}$");
    return std::regex_match(username, username_pattern);
}

bool AcademicSocialServer::validateEmail(const std::string& email) {
    return email.find('@') != std::string::npos && email.length() > 3;
}

} // namespace sohbet