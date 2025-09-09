#include "repositories/user_repository.h"
#include "db/database.h"
#include "models/user.h"
#include <iostream>
#include <cassert>

void test_user_repository_creation() {
    // Use in-memory database for testing
    sohbet::Database db(":memory:");
    sohbet::UserRepository repo(db);
    
    // Migrate database
    assert(repo.migrate());
    
    // Create test user
    sohbet::User user("test_student", "student@university.edu");
    user.setUniversity("Test University");
    user.setDepartment("Computer Science");
    user.setEnrollmentYear(2023);
    user.setPrimaryLanguage("Turkish");
    
    auto created_user = repo.create(user, "TestPassword123");
    assert(created_user.has_value());
    assert(created_user.value().getId().has_value());
    assert(created_user.value().getUsername() == "test_student");
    
    std::cout << "User repository creation test passed!" << std::endl;
}

void test_user_repository_find() {
    sohbet::Database db(":memory:");
    sohbet::UserRepository repo(db);
    repo.migrate();
    
    // Create test user
    sohbet::User user("jane_student", "jane@test.edu");
    auto created_user = repo.create(user, "SecurePass456");
    assert(created_user.has_value());
    
    // Find by username
    auto found_by_username = repo.findByUsername("jane_student");
    assert(found_by_username.has_value());
    assert(found_by_username.value().getEmail() == "jane@test.edu");
    
    // Find by email
    auto found_by_email = repo.findByEmail("jane@test.edu");
    assert(found_by_email.has_value());
    assert(found_by_email.value().getUsername() == "jane_student");
    
    // Test non-existent user
    auto not_found = repo.findByUsername("nonexistent");
    assert(!not_found.has_value());
    
    std::cout << "User repository find test passed!" << std::endl;
}

void test_user_repository_uniqueness() {
    sohbet::Database db(":memory:");
    sohbet::UserRepository repo(db);
    repo.migrate();
    
    // Create first user
    sohbet::User user1("unique_user", "unique@test.edu");
    auto created1 = repo.create(user1, "password123");
    assert(created1.has_value());
    
    // Try to create user with same username
    sohbet::User user2("unique_user", "different@test.edu");
    auto created2 = repo.create(user2, "password456");
    assert(!created2.has_value()); // Should fail due to username conflict
    
    // Try to create user with same email
    sohbet::User user3("different_user", "unique@test.edu");
    auto created3 = repo.create(user3, "password789");
    assert(!created3.has_value()); // Should fail due to email conflict
    
    // Check existence methods
    assert(repo.usernameExists("unique_user"));
    assert(!repo.usernameExists("nonexistent_user"));
    assert(repo.emailExists("unique@test.edu"));
    assert(!repo.emailExists("nonexistent@test.edu"));
    
    std::cout << "User repository uniqueness test passed!" << std::endl;
}

int main() {
    try {
        test_user_repository_creation();
        test_user_repository_find();
        test_user_repository_uniqueness();
        std::cout << "All user repository tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}