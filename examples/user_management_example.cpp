/**
 * Example program demonstrating C++ usage patterns for user management
 * 
 * This shows how to:
 * 1. Initialize the database with migrations
 * 2. Create users from form data
 * 3. Authenticate users with password verification
 * 4. Update user profiles
 * 5. Retrieve user information
 */

#include "init/database_initializer.h"
#include "helpers/user_helpers.h"
#include "repositories/user_repository.h"
#include <iostream>
#include <memory>
#include <optional>

// Example: Simulate user registration from a web form
std::optional<sohbet::User> registerUserFromForm(
    sohbet::helpers::UserHelpers& helpers,
    const std::string& form_username,
    const std::string& form_email,
    const std::string& form_password,
    const std::string& form_name,
    const std::string& form_university,
    const std::string& form_department,
    int form_enrollment_year
) {
    std::cout << "\n=== User Registration Example ===" << std::endl;
    std::cout << "Registering user: " << form_username << std::endl;

    auto user = helpers.createUser(
        form_username,
        form_email,
        form_password,
        form_name,
        form_university,
        form_department,
        form_enrollment_year
    );

    if (user.has_value()) {
        std::cout << "✓ User registered successfully!" << std::endl;
        std::cout << "  User ID: " << user->getId().value() << std::endl;
        std::cout << "  Username: " << user->getUsername() << std::endl;
        std::cout << "  Email: " << user->getEmail() << std::endl;
        std::cout << "  University: " << user->getUniversity().value_or("N/A") << std::endl;
    } else {
        std::cerr << "✗ Registration failed: " << helpers.getLastError() << std::endl;
    }

    return user;
}

// Example: Authenticate user from login form
std::optional<sohbet::User> loginUser(
    sohbet::helpers::UserHelpers& helpers,
    const std::string& login_username,
    const std::string& login_password
) {
    std::cout << "\n=== User Login Example ===" << std::endl;
    std::cout << "Attempting to authenticate: " << login_username << std::endl;

    auto user = helpers.authenticateUser(login_username, login_password);

    if (user.has_value()) {
        std::cout << "✓ Authentication successful!" << std::endl;
        std::cout << "  Welcome back, " << user->getName().value_or(user->getUsername()) << "!" << std::endl;
        std::cout << "  Role: " << user->getRole().value_or("Student") << std::endl;
    } else {
        std::cerr << "✗ Authentication failed: " << helpers.getLastError() << std::endl;
    }

    return user;
}

// Example: Update user profile from settings form
bool updateUserProfile(
    sohbet::helpers::UserHelpers& helpers,
    int user_id,
    const std::optional<std::string>& new_name,
    const std::optional<std::string>& new_phone,
    const std::optional<std::string>& new_language
) {
    std::cout << "\n=== User Profile Update Example ===" << std::endl;
    std::cout << "Updating profile for user ID: " << user_id << std::endl;

    bool success = helpers.updateUser(
        user_id,
        new_name,                    // name
        std::nullopt,                // position
        new_phone,                   // phone_number
        std::nullopt,                // university
        std::nullopt,                // department
        std::nullopt,                // enrollment_year
        new_language                 // primary_language
    );

    if (success) {
        std::cout << "✓ Profile updated successfully!" << std::endl;
    } else {
        std::cerr << "✗ Update failed: " << helpers.getLastError() << std::endl;
    }

    return success;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Sohbet User Management Example" << std::endl;
    std::cout << "========================================" << std::endl;

    // Step 1: Initialize database with migrations
    std::cout << "\n--- Step 1: Database Initialization ---" << std::endl;
    
    sohbet::init::DatabaseInitializer initializer("example.db", "migrations");
    
    if (!initializer.initialize()) {
        std::cerr << "Failed to initialize database: " << initializer.getLastError() << std::endl;
        return 1;
    }

    std::cout << "✓ Database initialized successfully" << std::endl;

    // Step 2: Create helper instances
    auto db = initializer.getDatabase();
    auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
    sohbet::helpers::UserHelpers helpers(user_repo);

    // Step 3: Example - Register a new user (like from a registration form)
    auto new_user = registerUserFromForm(
        helpers,
        "ali_yilmaz",                      // username
        "ali@university.edu.tr",            // email
        "SecurePassword123!",               // password
        "Ali Yılmaz",                      // full name
        "Istanbul Technical University",    // university
        "Computer Engineering",             // department
        2022                                // enrollment year
    );

    if (!new_user.has_value()) {
        std::cerr << "Cannot continue without a user" << std::endl;
        return 1;
    }

    int user_id = new_user->getId().value();

    // Step 4: Example - Try to login with correct password
    auto authenticated_user = loginUser(helpers, "ali_yilmaz", "SecurePassword123!");
    
    // Step 5: Example - Try to login with wrong password (should fail)
    std::cout << "\n--- Testing wrong password ---" << std::endl;
    auto failed_login = loginUser(helpers, "ali_yilmaz", "WrongPassword");
    
    // Step 6: Example - Update user profile
    updateUserProfile(
        helpers,
        user_id,
        "Ali Yılmaz (Updated)",    // new name
        "+90 555 123 4567",         // new phone
        "Turkish"                   // new language
    );

    // Step 7: Example - Retrieve updated user information
    std::cout << "\n=== Retrieve User Information ===" << std::endl;
    auto retrieved_user = helpers.getUserByUsername("ali_yilmaz");
    
    if (retrieved_user.has_value()) {
        std::cout << "User Profile:" << std::endl;
        std::cout << "  Username: " << retrieved_user->getUsername() << std::endl;
        std::cout << "  Email: " << retrieved_user->getEmail() << std::endl;
        std::cout << "  Name: " << retrieved_user->getName().value_or("N/A") << std::endl;
        std::cout << "  Phone: " << retrieved_user->getPhoneNumber().value_or("N/A") << std::endl;
        std::cout << "  University: " << retrieved_user->getUniversity().value_or("N/A") << std::endl;
        std::cout << "  Department: " << retrieved_user->getDepartment().value_or("N/A") << std::endl;
        std::cout << "  Language: " << retrieved_user->getPrimaryLanguage().value_or("N/A") << std::endl;
        std::cout << "  Role: " << retrieved_user->getRole().value_or("Student") << std::endl;
    }

    // Step 8: Example - Try to create duplicate user (should fail)
    std::cout << "\n--- Testing duplicate user creation ---" << std::endl;
    auto duplicate = registerUserFromForm(
        helpers,
        "ali_yilmaz",              // Same username (should fail)
        "different@email.com",
        "AnotherPassword123",
        "Different Person",
        "Different University",
        "Different Department",
        2023
    );

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Example completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
