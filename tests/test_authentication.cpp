#include "server/server.h"
#include "security/jwt.h"
#include <iostream>
#include <cassert>

void test_user_registration_login_retrieval() {
    // Create server with in-memory database
    sohbet::AcademicSocialServer server(":memory:");
    assert(server.initialize());
    
    // Test user registration
    std::string registration_request = R"({
        "username": "test_student",
        "email": "test@university.edu",
        "password": "SecurePassword123",
        "university": "Test University",
        "department": "Computer Science",
        "enrollment_year": 2023,
        "primary_language": "English"
    })";
    
    std::string registration_response = server.handleUserRegistration(registration_request);
    std::cout << "Registration response: " << registration_response << std::endl;
    
    // Should contain user data without password
    assert(registration_response.find("\"username\":\"test_student\"") != std::string::npos);
    assert(registration_response.find("\"email\":\"test@university.edu\"") != std::string::npos);
    assert(registration_response.find("password") == std::string::npos);
    assert(registration_response.find("\"id\":") != std::string::npos);
    
    // Test login with correct credentials
    std::string login_request = R"({
        "username": "test_student",
        "password": "SecurePassword123"
    })";
    
    std::string login_response = server.handleLogin(login_request);
    std::cout << "Login response: " << login_response << std::endl;
    
    // Should contain token and user data
    assert(login_response.find("\"token\":") != std::string::npos);
    assert(login_response.find("\"user\":") != std::string::npos);
    assert(login_response.find("\"username\":\"test_student\"") != std::string::npos);
    
    // Test login with wrong password
    std::string wrong_login_request = R"({
        "username": "test_student", 
        "password": "WrongPassword"
    })";
    
    std::string wrong_login_response = server.handleLogin(wrong_login_request);
    std::cout << "Wrong login response: " << wrong_login_response << std::endl;
    assert(wrong_login_response.find("\"error\":") != std::string::npos);
    assert(wrong_login_response.find("\"code\":401") != std::string::npos);
    
    // Test login with non-existent user
    std::string nonexistent_login_request = R"({
        "username": "nonexistent",
        "password": "AnyPassword"
    })";
    
    std::string nonexistent_login_response = server.handleLogin(nonexistent_login_request);
    std::cout << "Non-existent user login response: " << nonexistent_login_response << std::endl;
    assert(nonexistent_login_response.find("\"error\":") != std::string::npos);
    assert(nonexistent_login_response.find("\"code\":401") != std::string::npos);
    
    // Test user retrieval
    std::string user_retrieval_response = server.handleGetUser("test_student");
    std::cout << "User retrieval response: " << user_retrieval_response << std::endl;
    
    // Should contain user data without password
    assert(user_retrieval_response.find("\"username\":\"test_student\"") != std::string::npos);
    assert(user_retrieval_response.find("\"email\":\"test@university.edu\"") != std::string::npos);
    assert(user_retrieval_response.find("password") == std::string::npos);
    
    // Test retrieval of non-existent user
    std::string nonexistent_retrieval_response = server.handleGetUser("nonexistent");
    std::cout << "Non-existent user retrieval response: " << nonexistent_retrieval_response << std::endl;
    assert(nonexistent_retrieval_response.find("\"error\":") != std::string::npos);
    assert(nonexistent_retrieval_response.find("\"code\":404") != std::string::npos);
    
    std::cout << "All registration, login, and user retrieval tests passed!" << std::endl;
}

void test_jwt_functionality() {
    // Test JWT token generation and verification
    std::string token = sohbet::security::generate_jwt_token("testuser", 123);
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
    
    // Test with invalid token
    auto invalid_payload_opt = sohbet::security::verify_jwt_token("invalid.token.here");
    assert(!invalid_payload_opt.has_value());
    
    std::cout << "JWT functionality tests passed!" << std::endl;
}

int main() {
    try {
        test_jwt_functionality();
        test_user_registration_login_retrieval();
        std::cout << "All authentication tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}