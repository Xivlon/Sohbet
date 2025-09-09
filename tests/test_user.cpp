#include "models/user.h"
#include <iostream>
#include <cassert>

void test_user_json_serialization() {
    sohbet::User user("test_user", "test@example.com");
    user.setId(123);
    user.setUniversity("Test University");
    user.setDepartment("Computer Science");
    user.setEnrollmentYear(2023);
    user.setPrimaryLanguage("English");
    
    std::string json = user.toJson();
    std::cout << "Serialized JSON: " << json << std::endl;
    
    // Basic checks
    assert(json.find("\"username\":\"test_user\"") != std::string::npos);
    assert(json.find("\"email\":\"test@example.com\"") != std::string::npos);
    assert(json.find("\"id\":123") != std::string::npos);
    assert(json.find("password") == std::string::npos); // Password should not be in JSON
    
    std::cout << "User JSON serialization test passed!" << std::endl;
}

void test_user_json_deserialization() {
    std::string json = R"({"username":"jane_doe","email":"jane@university.edu","university":"Tech University","department":"Physics","enrollment_year":2022,"primary_language":"Turkish"})";
    
    sohbet::User user = sohbet::User::fromJson(json);
    
    assert(user.getUsername() == "jane_doe");
    assert(user.getEmail() == "jane@university.edu");
    assert(user.getUniversity().value() == "Tech University");
    assert(user.getDepartment().value() == "Physics");
    assert(user.getEnrollmentYear().value() == 2022);
    assert(user.getPrimaryLanguage().value() == "Turkish");
    
    std::cout << "User JSON deserialization test passed!" << std::endl;
}

int main() {
    try {
        test_user_json_serialization();
        test_user_json_deserialization();
        std::cout << "All user tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}