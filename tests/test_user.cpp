#include "models/user.h"
#include <iostream>
#include <cassert>

void testUserJsonSerialization() {
    std::cout << "Testing User JSON serialization..." << std::endl;
    
    // Create user
    sohbet::User user;
    user.setId(1);
    user.setUsername("test_user");
    user.setEmail("test@example.com");
    user.setUniversity("Test University");
    user.setDepartment("Computer Science");
    user.setEnrollmentYear(2023);
    user.setPrimaryLanguage("English");
    user.setAdditionalLanguages({"Turkish", "German"});
    user.setPasswordHash("secret_hash"); // Should not appear in JSON
    
    // Test JSON serialization
    std::string json = user.toJson();
    std::cout << "Serialized JSON: " << json << std::endl;
    
    // Verify password hash is not in JSON
    assert(json.find("password") == std::string::npos);
    assert(json.find("secret_hash") == std::string::npos);
    
    // Verify required fields are present
    assert(json.find("test_user") != std::string::npos);
    assert(json.find("test@example.com") != std::string::npos);
    assert(json.find("Test University") != std::string::npos);
    
    std::cout << "JSON serialization test passed!" << std::endl;
}

void testUserValidation() {
    std::cout << "Testing User validation..." << std::endl;
    
    // Test valid usernames
    assert(sohbet::User::isValidUsername("valid_user123"));
    assert(sohbet::User::isValidUsername("ABC"));
    assert(!sohbet::User::isValidUsername("ab")); // too short
    assert(!sohbet::User::isValidUsername("this_username_is_way_too_long_to_be_valid"));
    assert(!sohbet::User::isValidUsername("invalid-user")); // contains dash
    assert(!sohbet::User::isValidUsername("invalid user")); // contains space
    
    // Test valid emails
    assert(sohbet::User::isValidEmail("user@example.com"));
    assert(sohbet::User::isValidEmail("test@test.edu"));
    assert(!sohbet::User::isValidEmail("invalid-email"));
    assert(!sohbet::User::isValidEmail(""));
    
    // Test valid passwords
    assert(sohbet::User::isValidPassword("password123"));
    assert(sohbet::User::isValidPassword("12345678"));
    assert(!sohbet::User::isValidPassword("short"));
    assert(!sohbet::User::isValidPassword(""));
    
    std::cout << "User validation test passed!" << std::endl;
}

void testUserFromJson() {
    std::cout << "Testing User JSON deserialization..." << std::endl;
    
    std::string json = R"({
        "username": "json_user",
        "email": "json@example.com",
        "university": "JSON University",
        "department": "JSON Department",
        "enrollment_year": 2023,
        "primary_language": "JSON"
    })";
    
    sohbet::User user = sohbet::User::fromJson(json);
    
    assert(user.getUsername() == "json_user");
    assert(user.getEmail() == "json@example.com");
    assert(user.getUniversity() == "JSON University");
    assert(user.getDepartment() == "JSON Department");
    assert(user.getEnrollmentYear().has_value());
    assert(user.getEnrollmentYear().value() == 2023);
    assert(user.getPrimaryLanguage() == "JSON");
    
    std::cout << "JSON deserialization test passed!" << std::endl;
}

int main() {
    std::cout << "Running User model tests..." << std::endl;
    
    try {
        testUserJsonSerialization();
        testUserValidation();
        testUserFromJson();
        
        std::cout << "\nAll User tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}