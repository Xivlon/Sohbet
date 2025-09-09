#include "server/server.h"
#include "models/user.h"
#include "utils/hash.h"
#include <iostream>
#include <regex>

namespace sohbet {
namespace server {

AcademicSocialServer::AcademicSocialServer(int port, const std::string& db_path)
    : port_(port), db_path_(db_path) {
}

bool AcademicSocialServer::initialize() {
    // Initialize database
    database_ = std::make_shared<db::Database>(db_path_);
    if (!database_->isOpen()) {
        std::cerr << "Failed to open database: " << db_path_ << std::endl;
        return false;
    }
    
    // Initialize user repository
    user_repository_ = std::make_shared<repositories::UserRepository>(database_);
    
    // Run migrations
    if (!user_repository_->migrate()) {
        std::cerr << "Failed to run database migrations" << std::endl;
        return false;
    }
    
    std::cout << "Server initialized successfully" << std::endl;
    return true;
}

bool AcademicSocialServer::start() {
    std::cout << "Academic Social Server starting on port " << port_ << std::endl;
    std::cout << "Database: " << db_path_ << std::endl;
    std::cout << "Version: 0.2.0-academic" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /api/status" << std::endl;
    std::cout << "  GET  /api/users/demo" << std::endl;
    std::cout << "  POST /api/users" << std::endl;
    
    // In a real implementation, this would start an HTTP server
    // For now, we'll just indicate success
    std::cout << "Server ready to handle requests" << std::endl;
    return true;
}

HttpResponse AcademicSocialServer::handleRequest(const HttpRequest& request) {
    std::cout << "Processing request: " << request.method << " " << request.path << std::endl;
    
    if (request.method == "GET" && request.path == "/api/status") {
        return handleStatus(request);
    } else if (request.method == "GET" && request.path == "/api/users/demo") {
        return handleUsersDemo(request);
    } else if (request.method == "POST" && request.path == "/api/users") {
        return handleCreateUser(request);
    } else {
        return handleNotFound(request);
    }
}

HttpResponse AcademicSocialServer::handleStatus(const HttpRequest& request) {
    (void)request; // Suppress unused parameter warning
    
    std::string response = R"({
        "status": "ok",
        "version": "0.2.0-academic",
        "service": "Sohbet Academic Social Backend"
    })";
    
    return createJsonResponse(200, response);
}

HttpResponse AcademicSocialServer::handleUsersDemo(const HttpRequest& request) {
    (void)request; // Suppress unused parameter warning
    
    std::string response = R"({
        "message": "User registration endpoint available at POST /api/users",
        "example": {
            "username": "ali_student",
            "email": "ali@example.edu",
            "password": "StrongPass123",
            "university": "Istanbul Technical University",
            "department": "Computer Engineering",
            "enrollment_year": 2022,
            "primary_language": "Turkish",
            "additional_languages": ["English", "German"]
        }
    })";
    
    return createJsonResponse(200, response);
}

HttpResponse AcademicSocialServer::handleCreateUser(const HttpRequest& request) {
    try {
        // Extract required fields
        std::string username = extractJsonField(request.body, "username");
        std::string email = extractJsonField(request.body, "email");
        std::string password = extractJsonField(request.body, "password");
        
        // Validate required fields
        std::string error;
        if (!validateUserRegistration(username, email, password, error)) {
            return createErrorResponse(400, error);
        }
        
        // Check uniqueness
        if (user_repository_->usernameExists(username)) {
            return createErrorResponse(409, "Username already exists");
        }
        
        if (user_repository_->emailExists(email)) {
            return createErrorResponse(409, "Email already exists");
        }
        
        // Create user object
        User user;
        user.setUsername(username);
        user.setEmail(email);
        
        // Hash password
        std::string password_hash = utils::Hash::generateSaltedHash(password);
        user.setPasswordHash(password_hash);
        
        // Extract optional fields
        std::string university = extractJsonField(request.body, "university");
        if (!university.empty()) {
            user.setUniversity(university);
        }
        
        std::string department = extractJsonField(request.body, "department");
        if (!department.empty()) {
            user.setDepartment(department);
        }
        
        std::string enrollment_year_str = extractJsonField(request.body, "enrollment_year");
        if (!enrollment_year_str.empty()) {
            try {
                int year = std::stoi(enrollment_year_str);
                user.setEnrollmentYear(year);
            } catch (const std::exception&) {
                // Ignore invalid year format
            }
        }
        
        std::string primary_language = extractJsonField(request.body, "primary_language");
        if (!primary_language.empty()) {
            user.setPrimaryLanguage(primary_language);
        }
        
        // Parse additional_languages array (simplified)
        std::regex lang_regex("\"additional_languages\"\\s*:\\s*\\[(.*?)\\]");
        std::smatch match;
        if (std::regex_search(request.body, match, lang_regex)) {
            std::string langs_str = match[1].str();
            std::vector<std::string> languages;
            
            std::regex item_regex("\"([^\"]*)\"");
            std::sregex_iterator iter(langs_str.begin(), langs_str.end(), item_regex);
            std::sregex_iterator end;
            
            for (; iter != end; ++iter) {
                languages.push_back((*iter)[1].str());
            }
            
            user.setAdditionalLanguages(languages);
        }
        
        // Save to database
        if (!user_repository_->createUser(user)) {
            return createErrorResponse(500, "Failed to create user");
        }
        
        // Return created user (without password)
        return createJsonResponse(201, user.toJson());
        
    } catch (const std::exception& e) {
        return createErrorResponse(500, "Internal server error");
    }
}

HttpResponse AcademicSocialServer::handleNotFound(const HttpRequest& request) {
    (void)request; // Suppress unused parameter warning
    return createErrorResponse(404, "Endpoint not found");
}

HttpResponse AcademicSocialServer::createJsonResponse(int status_code, const std::string& json) {
    return HttpResponse(status_code, "application/json", json);
}

HttpResponse AcademicSocialServer::createErrorResponse(int status_code, const std::string& message) {
    std::string error_json = "{\"error\":\"" + message + "\"}";
    return HttpResponse(status_code, "application/json", error_json);
}

std::string AcademicSocialServer::extractJsonField(const std::string& json, const std::string& field) {
    std::regex field_regex("\"" + field + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, field_regex)) {
        return match[1].str();
    }
    
    // Try to match numeric fields
    std::regex numeric_regex("\"" + field + "\"\\s*:\\s*(\\d+)");
    if (std::regex_search(json, match, numeric_regex)) {
        return match[1].str();
    }
    
    return "";
}

bool AcademicSocialServer::validateUserRegistration(const std::string& username, const std::string& email, const std::string& password, std::string& error) {
    if (username.empty()) {
        error = "Username is required";
        return false;
    }
    
    if (!User::isValidUsername(username)) {
        error = "Username must be 3-32 characters long and contain only letters, numbers, and underscores";
        return false;
    }
    
    if (email.empty()) {
        error = "Email is required";
        return false;
    }
    
    if (!User::isValidEmail(email)) {
        error = "Email must contain @ symbol";
        return false;
    }
    
    if (password.empty()) {
        error = "Password is required";
        return false;
    }
    
    if (!User::isValidPassword(password)) {
        error = "Password must be at least 8 characters long";
        return false;
    }
    
    return true;
}

} // namespace server
} // namespace sohbet