#include "server/server.h"
#include "models/user.h"
#include "security/jwt.h"
#include "utils/hash.h"
#include <iostream>
#include <regex>
#include <sstream>

namespace sohbet {
namespace server {

AcademicSocialServer::AcademicSocialServer(int port, const std::string& db_path)
    : port_(port), db_path_(db_path) {
}

bool AcademicSocialServer::initialize() {
    database_ = std::make_shared<db::Database>(db_path_);
    if (!database_->isOpen()) {
        std::cerr << "Failed to open database: " << db_path_ << std::endl;
        return false;
    }

    user_repository_ = std::make_shared<repositories::UserRepository>(database_);

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
    std::cout << "Server ready to handle requests" << std::endl;
    return true;
}

// -------------------- Request Handlers --------------------

HttpResponse AcademicSocialServer::handleRequest(const HttpRequest& request) {
    if (request.method == "GET" && request.path == "/api/status") {
        return handleStatus(request);
    } else if (request.method == "GET" && request.path == "/api/users/demo") {
        return handleUsersDemo(request);
    } else if (request.method == "POST" && request.path == "/api/users") {
        return handleCreateUser(request);
    } else if (request.method == "POST" && request.path == "/api/login") {
        return handleLogin(request);
    } else {
        return handleNotFound(request);
    }
}

HttpResponse AcademicSocialServer::handleStatus(const HttpRequest& request) {
    (void)request;
    std::string response = R"({"status":"ok","version":"0.2.0-academic","features":["user_registration","sqlite_persistence","bcrypt_hashing"]})";
    return createJsonResponse(200, response);
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

        if (!user_repository_->createUser(user)) {
            return createErrorResponse(500, "Failed to create user");
        }

        return createJsonResponse(201, user.toJson());
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

        std::string token = security::generate_jwt_token(username, user.getId().value());

        std::ostringstream oss;
        oss << "{ \"token\":\"" << token << "\", \"user\":" << user.toJson() << " }";
        return createJsonResponse(200, oss.str());
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

} // namespace server
} // namespace sohbet