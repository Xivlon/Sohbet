#include "server/server.h"
#include <iostream>

void testValidationScenarios(sohbet::server::AcademicSocialServer& server) {
    std::cout << "\n=== Testing Validation Scenarios ===" << std::endl;
    
    // Test cases
    struct TestCase {
        std::string name;
        std::string json;
        int expected_status;
    };
    
    std::vector<TestCase> tests = {
        {
            "Valid Registration",
            R"({"username":"test_user","email":"test@test.edu","password":"ValidPass123","university":"Test University"})",
            201
        },
        {
            "Invalid Username (too short)",
            R"({"username":"ab","email":"test1@test.edu","password":"ValidPass123"})",
            400
        },
        {
            "Invalid Password (too short)",
            R"({"username":"test_user2","email":"test2@test.edu","password":"short"})",
            400
        },
        {
            "Invalid Email (no @)",
            R"({"username":"test_user3","email":"invalid-email","password":"ValidPass123"})",
            400
        },
        {
            "Duplicate Username",
            R"({"username":"test_user","email":"different@test.edu","password":"ValidPass123"})",
            409
        },
        {
            "User with All Fields",
            R"({"username":"full_user","email":"full@test.edu","password":"FullPass123","university":"Full University","department":"Computer Science","enrollment_year":2023,"primary_language":"English","additional_languages":["Turkish","German"]})",
            201
        }
    };
    
    for (const auto& test : tests) {
        std::cout << "\nTest: " << test.name << std::endl;
        std::cout << "Data: " << test.json << std::endl;
        
        sohbet::server::HttpRequest request("POST", "/api/users", test.json);
        auto response = server.handleRequest(request);
        
        std::cout << "Expected: " << test.expected_status 
                  << " | Actual: " << response.status_code;
        
        if (response.status_code == test.expected_status) {
            std::cout << " ✅ PASS" << std::endl;
        } else {
            std::cout << " ❌ FAIL" << std::endl;
        }
        
        std::cout << "Response: " << response.body << std::endl;
        std::cout << "---" << std::endl;
    }
}

int main() {
    std::cout << "Starting Sohbet Validation Test Suite" << std::endl;
    
    // Create server instance with in-memory database for testing
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    
    // Initialize server
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    // Run validation tests
    testValidationScenarios(server);
    
    std::cout << "\nValidation test suite completed!" << std::endl;
    return 0;
}