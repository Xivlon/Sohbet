#include "server/server.h"
#include "security/jwt.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

void test_user_registration_login_retrieval() {
    // Set JWT secret for testing
    setenv("SOHBET_JWT_SECRET", "test_secret_key_for_unit_tests_only", 1);
    
    // Create server with in-memory database
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    assert(server.initialize());
    
    // Test user registration
    std::string registration_body = R"({
        "username": "test_student",
        "email": "test@university.edu",
        "password": "SecurePassword123",
        "university": "Test University",
        "department": "Computer Science",
        "enrollment_year": 2023,
        "primary_language": "English"
    })";
    
    sohbet::server::HttpRequest reg_request("POST", "/api/users", registration_body);
    auto registration_response = server.handleRequest(reg_request);
    std::cout << "Registration response: " << registration_response.body << std::endl;
    
    // Should contain user data without password
    assert(registration_response.body.find("\"username\":\"test_student\"") != std::string::npos);
    assert(registration_response.body.find("\"email\":\"test@university.edu\"") != std::string::npos);
    assert(registration_response.body.find("password") == std::string::npos);
    assert(registration_response.body.find("\"id\":") != std::string::npos);
    
    // Test login with correct credentials
    std::string login_body = R"({
        "username": "test_student",
        "password": "SecurePassword123"
    })";
    
    sohbet::server::HttpRequest login_request("POST", "/api/login", login_body);
    auto login_response = server.handleRequest(login_request);
    std::cout << "Login response: " << login_response.body << std::endl;
    
    // Should contain token and user data
    assert(login_response.body.find("\"token\":") != std::string::npos);
    assert(login_response.body.find("\"user\":") != std::string::npos);
    assert(login_response.body.find("\"username\":\"test_student\"") != std::string::npos);
    
    // Test login with wrong password
    std::string wrong_login_body = R"({
        "username": "test_student", 
        "password": "WrongPassword"
    })";
    
    sohbet::server::HttpRequest wrong_login_request("POST", "/api/login", wrong_login_body);
    auto wrong_login_response = server.handleRequest(wrong_login_request);
    std::cout << "Wrong login response: " << wrong_login_response.body << std::endl;
    assert(wrong_login_response.body.find("\"error\":") != std::string::npos);
    assert(wrong_login_response.status_code == 401);
    
    // Test login with non-existent user
    std::string nonexistent_login_body = R"({
        "username": "nonexistent",
        "password": "AnyPassword"
    })";
    
    sohbet::server::HttpRequest nonexistent_login_request("POST", "/api/login", nonexistent_login_body);
    auto nonexistent_login_response = server.handleRequest(nonexistent_login_request);
    std::cout << "Non-existent user login response: " << nonexistent_login_response.body << std::endl;
    assert(nonexistent_login_response.body.find("\"error\":") != std::string::npos);
    assert(nonexistent_login_response.status_code == 401);
    
    // Test demo user endpoint (since there's no general user retrieval endpoint in current server)
    sohbet::server::HttpRequest demo_request("GET", "/api/users/demo", "");
    auto demo_response = server.handleRequest(demo_request);
    std::cout << "Demo user response: " << demo_response.body << std::endl;
    
    // Should contain demo user data
    assert(demo_response.body.find("\"username\":") != std::string::npos);
    assert(demo_response.body.find("password") == std::string::npos);
    
    std::cout << "All registration, login, and user retrieval tests passed!" << std::endl;
}

void test_jwt_functionality() {
    // Test JWT token generation and verification
    std::string token = sohbet::security::generate_jwt_token("testuser", 123, "student");
    std::cout << "Generated JWT token: " << token << std::endl;
    
    // Token should be non-empty and contain dots
    assert(!token.empty());
    assert(token.find('.') != std::string::npos);
    
    // Verify token
    auto payload_opt = sohbet::security::verify_jwt_token(token);
    assert(payload_opt.has_value());
    
    auto payload = payload_opt.value();
    assert(payload.username == "testuser");
    assert(payload.user_id == 123);
    assert(payload.role == "student");
    
    // Test with invalid token
    auto invalid_payload_opt = sohbet::security::verify_jwt_token("invalid.token.here");
    assert(!invalid_payload_opt.has_value());
    
    std::cout << "JWT functionality tests passed!" << std::endl;
}

void test_demo_user_authentication() {
    std::cout << "Testing demo user authentication..." << std::endl;
    
    // Set JWT secret for testing
    setenv("SOHBET_JWT_SECRET", "test_secret_key_for_unit_tests_only", 1);
    
    // Create server with in-memory database
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    assert(server.initialize());
    
    // Test demo user login with correct credentials
    std::string demo_login_body = R"({
        "username": "demo_student",
        "password": "demo123"
    })";
    
    sohbet::server::HttpRequest demo_login_request("POST", "/api/login", demo_login_body);
    auto demo_login_response = server.handleRequest(demo_login_request);
    std::cout << "Demo login response: " << demo_login_response.body << std::endl;
    
    // Should contain token and user data
    assert(demo_login_response.status_code == 200);
    assert(demo_login_response.body.find("\"token\":") != std::string::npos);
    assert(demo_login_response.body.find("\"user\":") != std::string::npos);
    assert(demo_login_response.body.find("\"username\":\"demo_student\"") != std::string::npos);
    assert(demo_login_response.body.find("\"university\":\"Demo University\"") != std::string::npos);
    
    // Test demo user login with incorrect credentials
    std::string wrong_login_body = R"({
        "username": "demo_student",
        "password": "wrong_password"
    })";
    
    sohbet::server::HttpRequest wrong_login_request("POST", "/api/login", wrong_login_body);
    auto wrong_login_response = server.handleRequest(wrong_login_request);
    std::cout << "Wrong password response: " << wrong_login_response.body << std::endl;
    
    // Should fail with 401
    assert(wrong_login_response.status_code == 401);
    assert(wrong_login_response.body.find("\"error\":") != std::string::npos);
    
    std::cout << "Demo user authentication tests passed!" << std::endl;
}

int main() {
    try {
        test_jwt_functionality();
        test_user_registration_login_retrieval();
        test_demo_user_authentication();
        std::cout << "All authentication tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}