#include "repositories/user_repository.h"
#include "utils/hash.h"
#include "models/user.h"
#include <iostream>
#include <cassert>
#include <memory>

void testUserRepositoryMigration() {
    std::cout << "Testing UserRepository migration..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::repositories::UserRepository repo(db);
    assert(repo.migrate());

    std::cout << "Migration test passed!" << std::endl;
}

void testUserCreationAndRetrieval() {
    std::cout << "Testing User creation and retrieval..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::repositories::UserRepository repo(db);
    assert(repo.migrate());

    // Create test user
    sohbet::User user;
    user.setUsername("test_user");
    user.setEmail("test@example.com");
    user.setPasswordHash(sohbet::utils::Hash::generateSaltedHash("testpassword"));
    user.setUniversity("Test University");
    user.setDepartment("Computer Science");
    user.setEnrollmentYear(2022);
    user.setPrimaryLanguage("English");
    user.setAdditionalLanguages({"Turkish", "German"});

    // Save user
    assert(repo.createUser(user));
    assert(user.getId().has_value());
    assert(user.getId().value() > 0);

    // Retrieve by username
    auto found_user = repo.findByUsername("test_user");
    assert(found_user.has_value());
    assert(found_user->getUsername() == "test_user");
    assert(found_user->getEmail() == "test@example.com");
    assert(found_user->getUniversity() == "Test University");
    assert(found_user->getDepartment() == "Computer Science");
    assert(found_user->getEnrollmentYear() == 2022);
    assert(found_user->getPrimaryLanguage() == "English");

    // Check additional languages
    const auto& langs = found_user->getAdditionalLanguages();
    assert(langs.size() == 2);
    assert(langs[0] == "Turkish");
    assert(langs[1] == "German");

    // Retrieve by email
    auto found_by_email = repo.findByEmail("test@example.com");
    assert(found_by_email.has_value());
    assert(found_by_email->getUsername() == "test_user");

    // Retrieve by ID
    auto found_by_id = repo.findById(user.getId().value());
    assert(found_by_id.has_value());
    assert(found_by_id->getUsername() == "test_user");

    std::cout << "User creation and retrieval test passed!" << std::endl;
}

void testUniqueConstraints() {
    std::cout << "Testing unique constraints..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::repositories::UserRepository repo(db);
    assert(repo.migrate());

    // Create first user
    sohbet::User user1;
    user1.setUsername("unique_user");
    user1.setEmail("unique@example.com");
    user1.setPasswordHash("hash1");
    assert(repo.createUser(user1));

    // Try to create user with same username
    sohbet::User user2;
    user2.setUsername("unique_user"); // Same username
    user2.setEmail("different@example.com");
    user2.setPasswordHash("hash2");
    assert(!repo.createUser(user2)); // Should fail due to unique constraint

    // Try to create user with same email
    sohbet::User user3;
    user3.setUsername("different_user");
    user3.setEmail("unique@example.com"); // Same email
    user3.setPasswordHash("hash3");
    assert(!repo.createUser(user3)); // Should fail due to unique constraint

    // Check existence methods
    assert(repo.usernameExists("unique_user"));
    assert(!repo.usernameExists("nonexistent_user"));
    assert(repo.emailExists("unique@example.com"));
    assert(!repo.emailExists("nonexistent@example.com"));

    std::cout << "Unique constraints test passed!" << std::endl;
}

void testPasswordHashing() {
    std::cout << "Testing password hashing..." << std::endl;

    std::string password = "testpassword123";
    std::string salted_hash = sohbet::utils::Hash::generateSaltedHash(password);

    assert(!salted_hash.empty());
    assert(salted_hash.find(':') != std::string::npos);

    // Verify correct password
    assert(sohbet::utils::Hash::verifySaltedHash(password, salted_hash));

    // Verify incorrect password
    assert(!sohbet::utils::Hash::verifySaltedHash("wrongpassword", salted_hash));

    std::cout << "Password hashing test passed!" << std::endl;
}

int main() {
    std::cout << "Running UserRepository tests..." << std::endl;

    try {
        testUserRepositoryMigration();
        testUserCreationAndRetrieval();
        testUniqueConstraints();
        testPasswordHashing();

        std::cout << "\nAll UserRepository tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}