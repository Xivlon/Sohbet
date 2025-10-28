#include "helpers/user_helpers.h"
#include "repositories/user_repository.h"
#include "db/database.h"
#include <iostream>
#include <cassert>
#include <memory>

void testCreateUser() {
    std::cout << "Testing createUser helper..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
    assert(user_repo->migrate());

    sohbet::helpers::UserHelpers helpers(user_repo);

    // Test successful user creation
    auto user = helpers.createUser(
        "testuser",
        "test@example.com",
        "password123",
        "Test User",
        "Test University",
        "Computer Science",
        2023
    );

    assert(user.has_value());
    assert(user->getUsername() == "testuser");
    assert(user->getEmail() == "test@example.com");
    assert(user->getName() == "Test User");
    assert(user->getUniversity() == "Test University");
    assert(user->getDepartment() == "Computer Science");
    assert(user->getEnrollmentYear() == 2023);
    assert(user->getId().has_value());
    assert(user->getId().value() > 0);

    // Test duplicate username
    auto duplicate_user = helpers.createUser(
        "testuser",  // Same username
        "different@example.com",
        "password456"
    );
    assert(!duplicate_user.has_value());
    assert(helpers.getLastError().find("already exists") != std::string::npos);

    // Test invalid username
    auto invalid_user = helpers.createUser(
        "ab",  // Too short
        "test2@example.com",
        "password789"
    );
    assert(!invalid_user.has_value());
    assert(helpers.getLastError().find("Invalid username") != std::string::npos);

    // Test invalid password
    auto weak_password = helpers.createUser(
        "testuser2",
        "test2@example.com",
        "weak"  // Too short
    );
    assert(!weak_password.has_value());
    assert(helpers.getLastError().find("Invalid password") != std::string::npos);

    std::cout << "createUser helper test passed!" << std::endl;
}

void testGetUserByUsername() {
    std::cout << "Testing getUserByUsername helper..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
    assert(user_repo->migrate());

    sohbet::helpers::UserHelpers helpers(user_repo);

    // Create a user first
    auto created = helpers.createUser("findme", "find@example.com", "password123");
    assert(created.has_value());

    // Test finding existing user
    auto found = helpers.getUserByUsername("findme");
    assert(found.has_value());
    assert(found->getUsername() == "findme");
    assert(found->getEmail() == "find@example.com");

    // Test finding non-existent user
    auto not_found = helpers.getUserByUsername("nonexistent");
    assert(!not_found.has_value());
    assert(helpers.getLastError().find("not found") != std::string::npos);

    std::cout << "getUserByUsername helper test passed!" << std::endl;
}

void testUpdateUser() {
    std::cout << "Testing updateUser helper..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
    assert(user_repo->migrate());

    sohbet::helpers::UserHelpers helpers(user_repo);

    // Create a user
    auto created = helpers.createUser("updateme", "update@example.com", "password123");
    assert(created.has_value());
    int user_id = created->getId().value();

    // Update user profile
    bool success = helpers.updateUser(
        user_id,
        "Updated Name",
        "Professor",
        "+1234567890",
        "Updated University",
        "Updated Department",
        2024,
        "Turkish"
    );
    assert(success);

    // Verify updates
    auto updated = helpers.getUserById(user_id);
    assert(updated.has_value());
    assert(updated->getName() == "Updated Name");
    assert(updated->getPosition() == "Professor");
    assert(updated->getPhoneNumber() == "+1234567890");
    assert(updated->getUniversity() == "Updated University");
    assert(updated->getDepartment() == "Updated Department");
    assert(updated->getEnrollmentYear() == 2024);
    assert(updated->getPrimaryLanguage() == "Turkish");

    // Test updating non-existent user
    bool fail = helpers.updateUser(99999, "Should Fail");
    assert(!fail);
    assert(helpers.getLastError().find("not found") != std::string::npos);

    std::cout << "updateUser helper test passed!" << std::endl;
}

void testAuthenticateUser() {
    std::cout << "Testing authenticateUser helper..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
    assert(user_repo->migrate());

    sohbet::helpers::UserHelpers helpers(user_repo);

    // Create a user
    auto created = helpers.createUser("authtest", "auth@example.com", "correctpassword");
    assert(created.has_value());

    // Test successful authentication
    auto authenticated = helpers.authenticateUser("authtest", "correctpassword");
    assert(authenticated.has_value());
    assert(authenticated->getUsername() == "authtest");
    assert(authenticated->getEmail() == "auth@example.com");

    // Test wrong password
    auto wrong_pass = helpers.authenticateUser("authtest", "wrongpassword");
    assert(!wrong_pass.has_value());
    assert(helpers.getLastError().find("Invalid") != std::string::npos);

    // Test non-existent user
    auto no_user = helpers.authenticateUser("nonexistent", "anypassword");
    assert(!no_user.has_value());
    assert(helpers.getLastError().find("Invalid") != std::string::npos);

    std::cout << "authenticateUser helper test passed!" << std::endl;
}

int main() {
    std::cout << "Running User Helpers tests..." << std::endl;

    try {
        testCreateUser();
        testGetUserByUsername();
        testUpdateUser();
        testAuthenticateUser();

        std::cout << "\nAll User Helpers tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
