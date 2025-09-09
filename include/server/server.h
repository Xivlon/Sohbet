#pragma once
#include "db/database.h"
#include "repositories/user_repository.h"
#include <memory>
#include <string>

namespace sohbet {

class AcademicSocialServer {
public:
    AcademicSocialServer(const std::string& db_path = "academic.db");
    ~AcademicSocialServer() = default;
    
    // Initialize server and database
    bool initialize();
    
    // Start server (simplified for this implementation)
    void run(int port = 8080);
    
    // HTTP request handlers
    std::string handleStatusRequest();
    std::string handleUsersDemoRequest();
    std::string handleUserRegistration(const std::string& request_body);
    std::string handleLogin(const std::string& request_body);
    std::string handleGetUser(const std::string& username);

private:
    std::unique_ptr<Database> db_;
    std::unique_ptr<UserRepository> user_repository_;
    
    // Helper methods
    std::string jsonError(const std::string& message, int code = 400);
    bool validateUsername(const std::string& username);
    bool validateEmail(const std::string& email);
};

} // namespace sohbet