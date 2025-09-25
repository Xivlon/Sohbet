#pragma once

#include "db/database.h"
#include "repositories/user_repository.h"
#include <memory>
#include <string>
#include <thread>
#include <atomic>

namespace sohbet {
namespace server {

/**
 * HTTP Response structure
 */
struct HttpResponse {
    int status_code;
    std::string content_type;
    std::string body;
    
    HttpResponse(int code, const std::string& type, const std::string& content)
        : status_code(code), content_type(type), body(content) {}
};

/**
 * HTTP Request structure (simplified)
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    
    HttpRequest(const std::string& m, const std::string& p, const std::string& b)
        : method(m), path(p), body(b) {}
};

/**
 * Academic Social Server
 * Simple HTTP server for the academic social platform
 */
class AcademicSocialServer {
public:
    /**
     * Constructor
     * @param port Port to listen on
     * @param db_path Path to SQLite database file
     */
    AcademicSocialServer(int port = 8080, const std::string& db_path = "academic.db");
    
    /**
     * Destructor
     */
    ~AcademicSocialServer() = default;
    
    /**
     * Initialize the server (setup database, run migrations)
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * Start the server (blocking call)
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * Stop the server gracefully
     */
    void stop();
    
    /**
     * Process HTTP request (for testing/simulation)
     * @param request HTTP request to process
     * @return HTTP response
     */
    HttpResponse handleRequest(const HttpRequest& request);

private:
    int port_;
    std::string db_path_;
    std::shared_ptr<db::Database> database_;
    std::shared_ptr<repositories::UserRepository> user_repository_;
    std::atomic<bool> running_;
    int server_socket_;
    
    // HTTP server methods
    bool initializeSocket();
    void handleClient(int client_socket);
    HttpRequest parseHttpRequest(const std::string& raw_request);
    std::string formatHttpResponse(const HttpResponse& response);
    
    // Route handlers
    HttpResponse handleStatus(const HttpRequest& request);
    HttpResponse handleUsersDemo(const HttpRequest& request);
    HttpResponse handleCreateUser(const HttpRequest& request);
    HttpResponse handleLogin(const HttpRequest& request);
    HttpResponse handleNotFound(const HttpRequest& request);
    
    // Helper methods
    HttpResponse createJsonResponse(int status_code, const std::string& json);
    HttpResponse createErrorResponse(int status_code, const std::string& message);
    std::string extractJsonField(const std::string& json, const std::string& field);
    bool validateUserRegistration(const std::string& username, const std::string& email, const std::string& password, std::string& error);
    void ensureDemoUserExists();
};

} // namespace server
} // namespace sohbet