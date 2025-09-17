#include "server/server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

// Global server instance for signal handling
sohbet::server::AcademicSocialServer* global_server = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
    exit(0);
}

int main() {
    std::cout << "Starting Sohbet Academic Social Backend v0.2.0-academic" << std::endl;

    // Create server instance on port 8080 with database file
    sohbet::server::AcademicSocialServer server(8080, "academic.db");
    global_server = &server;
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize server (setup database, run migrations)
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    // Test API endpoints before starting HTTP server
    std::cout << "\n--- Testing API endpoints ---" << std::endl;

    // Status endpoint
    sohbet::server::HttpRequest status_req("GET", "/api/status", "");
    auto status_resp = server.handleRequest(status_req);
    std::cout << "\nGET /api/status -> " << status_resp.status_code << std::endl;
    std::cout << status_resp.body << std::endl;

    // Demo users endpoint
    sohbet::server::HttpRequest demo_req("GET", "/api/users/demo", "");
    auto demo_resp = server.handleRequest(demo_req);
    std::cout << "\nGET /api/users/demo -> " << demo_resp.status_code << std::endl;
    std::cout << demo_resp.body << std::endl;

    // User registration endpoint
    std::string registration_body = R"({
        "username": "ali_student",
        "email": "ali@example.edu",
        "password": "StrongPass123",
        "university": "Istanbul Technical University",
        "department": "Computer Engineering",
        "enrollment_year": 2022,
        "primary_language": "Turkish",
        "additional_languages": ["English", "German"]
    })";

    sohbet::server::HttpRequest reg_req("POST", "/api/users", registration_body);
    auto reg_resp = server.handleRequest(reg_req);
    std::cout << "\nPOST /api/users -> " << reg_resp.status_code << std::endl;
    std::cout << reg_resp.body << std::endl;

    std::cout << "\nServer demo completed successfully!" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
    
    // Start the HTTP server (blocking call)
    if (!server.start()) {
        std::cerr << "Failed to start HTTP server" << std::endl;
        return 1;
    }

    return 0;
}