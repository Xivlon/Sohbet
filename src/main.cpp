#include "server/server.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Sohbet Academic Social Backend v0.2.0-academic" << std::endl;

    // Create server instance on port 8080 with database file
    sohbet::server::AcademicSocialServer server(8080, "academic.db");

    // Initialize server (setup database, run migrations)
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }

    // Start server (blocking or simulated run)
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    // Demonstration of API endpoints
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
    
    // For demonstration purposes, let's create a simple HTTP server simulation
    std::cout << "\n🌐 Starting HTTP server simulation..." << std::endl;
    std::cout << "Server listening on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /api/status" << std::endl;
    std::cout << "  GET  /api/users/demo" << std::endl;
    std::cout << "  POST /api/users" << std::endl;
    std::cout << "  POST /api/login" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
    
    // Keep the server running for demonstration
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}