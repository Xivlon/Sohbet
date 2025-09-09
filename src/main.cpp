#include "server/server.h"
#include <iostream>

int main() {
    std::cout << "Starting Sohbet Academic Social Backend v0.2.0-academic" << std::endl;
    
    // Create server instance
    sohbet::server::AcademicSocialServer server(8080, "academic.db");
    
    // Initialize server
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    // Start server
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    // In a real implementation, this would run an event loop
    // For now, we'll just demonstrate the API with some test requests
    std::cout << "\n--- Testing API endpoints ---" << std::endl;
    
    // Test status endpoint
    sohbet::server::HttpRequest status_req("GET", "/api/status", "");
    auto status_resp = server.handleRequest(status_req);
    std::cout << "\nGET /api/status -> " << status_resp.status_code << std::endl;
    std::cout << status_resp.body << std::endl;
    
    // Test demo endpoint
    sohbet::server::HttpRequest demo_req("GET", "/api/users/demo", "");
    auto demo_resp = server.handleRequest(demo_req);
    std::cout << "\nGET /api/users/demo -> " << demo_resp.status_code << std::endl;
    std::cout << demo_resp.body << std::endl;
    
    // Test user registration endpoint
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
    return 0;
}