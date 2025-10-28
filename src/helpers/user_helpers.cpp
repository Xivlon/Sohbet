#include "helpers/user_helpers.h"
#include "utils/hash.h"
#include <iostream>

namespace sohbet {
namespace helpers {

UserHelpers::UserHelpers(std::shared_ptr<repositories::UserRepository> user_repository)
    : user_repository_(user_repository) {}

std::optional<User> UserHelpers::createUser(
    const std::string& username,
    const std::string& email,
    const std::string& password,
    const std::optional<std::string>& name,
    const std::optional<std::string>& university,
    const std::optional<std::string>& department,
    const std::optional<int>& enrollment_year
) {
    clearError();

    // Validate username
    if (!User::isValidUsername(username)) {
        setError("Invalid username: must be 3-32 alphanumeric characters");
        return std::nullopt;
    }

    // Validate email
    if (!User::isValidEmail(email)) {
        setError("Invalid email address");
        return std::nullopt;
    }

    // Validate password
    if (!User::isValidPassword(password)) {
        setError("Invalid password: must be at least 8 characters");
        return std::nullopt;
    }

    // Check if username already exists
    if (user_repository_->usernameExists(username)) {
        setError("Username already exists");
        return std::nullopt;
    }

    // Check if email already exists
    if (user_repository_->emailExists(email)) {
        setError("Email already exists");
        return std::nullopt;
    }

    // Create user object
    User user(username, email);
    
    if (name.has_value()) {
        user.setName(name);
    }
    if (university.has_value()) {
        user.setUniversity(university);
    }
    if (department.has_value()) {
        user.setDepartment(department);
    }
    if (enrollment_year.has_value()) {
        user.setEnrollmentYear(enrollment_year);
    }

    // Set default role to Student
    user.setRole("Student");

    // Create user with password hashing (handled by repository)
    auto created_user = user_repository_->create(user, password);
    
    if (!created_user.has_value()) {
        setError("Failed to create user in database");
        return std::nullopt;
    }

    return created_user;
}

std::optional<User> UserHelpers::getUserByUsername(const std::string& username) {
    clearError();

    auto user = user_repository_->findByUsername(username);
    
    if (!user.has_value()) {
        setError("User not found");
    }

    return user;
}

std::optional<User> UserHelpers::getUserById(int user_id) {
    clearError();

    auto user = user_repository_->findById(user_id);
    
    if (!user.has_value()) {
        setError("User not found");
    }

    return user;
}

bool UserHelpers::updateUser(
    int user_id,
    const std::optional<std::string>& name,
    const std::optional<std::string>& position,
    const std::optional<std::string>& phone_number,
    const std::optional<std::string>& university,
    const std::optional<std::string>& department,
    const std::optional<int>& enrollment_year,
    const std::optional<std::string>& primary_language
) {
    clearError();

    // First, get the existing user
    auto existing_user = user_repository_->findById(user_id);
    if (!existing_user.has_value()) {
        setError("User not found");
        return false;
    }

    // Update fields if provided
    User updated_user = existing_user.value();
    
    if (name.has_value()) {
        updated_user.setName(name);
    }
    if (position.has_value()) {
        updated_user.setPosition(position);
    }
    if (phone_number.has_value()) {
        updated_user.setPhoneNumber(phone_number);
    }
    if (university.has_value()) {
        updated_user.setUniversity(university);
    }
    if (department.has_value()) {
        updated_user.setDepartment(department);
    }
    if (enrollment_year.has_value()) {
        updated_user.setEnrollmentYear(enrollment_year);
    }
    if (primary_language.has_value()) {
        updated_user.setPrimaryLanguage(primary_language);
    }

    // Perform update
    bool success = user_repository_->update(updated_user);
    
    if (!success) {
        setError("Failed to update user in database");
    }

    return success;
}

std::optional<User> UserHelpers::authenticateUser(
    const std::string& username,
    const std::string& password
) {
    clearError();

    // Find user by username
    auto user = user_repository_->findByUsername(username);
    
    if (!user.has_value()) {
        setError("Invalid username or password");
        return std::nullopt;
    }

    // Verify password using bcrypt
    bool password_valid = utils::verify_password(password, user->getPasswordHash());
    
    if (!password_valid) {
        setError("Invalid username or password");
        return std::nullopt;
    }

    return user;
}

} // namespace helpers
} // namespace sohbet
