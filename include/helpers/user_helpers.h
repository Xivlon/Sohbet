#pragma once

#include "models/user.h"
#include "repositories/user_repository.h"
#include <memory>
#include <optional>
#include <string>

namespace sohbet {
namespace helpers {

/**
 * High-level user helper functions that provide convenient wrappers
 * around the UserRepository with built-in validation and error handling.
 * These functions integrate password hashing automatically.
 */
class UserHelpers {
public:
    /**
     * Constructor
     * @param user_repository Shared pointer to UserRepository instance
     */
    explicit UserHelpers(std::shared_ptr<repositories::UserRepository> user_repository);

    /**
     * Create a new user with automatic password hashing and validation
     * @param username User's username (3-32 alphanumeric characters)
     * @param email User's email address
     * @param password Plain text password (minimum 8 characters)
     * @param name Optional full name
     * @param university Optional university name
     * @param department Optional department name
     * @param enrollment_year Optional enrollment year
     * @return Created User object if successful, nullopt otherwise with error message set
     */
    std::optional<User> createUser(
        const std::string& username,
        const std::string& email,
        const std::string& password,
        const std::optional<std::string>& name = std::nullopt,
        const std::optional<std::string>& university = std::nullopt,
        const std::optional<std::string>& department = std::nullopt,
        const std::optional<int>& enrollment_year = std::nullopt
    );

    /**
     * Get a user by username
     * @param username Username to search for
     * @return User object if found, nullopt otherwise
     */
    std::optional<User> getUserByUsername(const std::string& username);

    /**
     * Get a user by ID
     * @param user_id User ID to search for
     * @return User object if found, nullopt otherwise
     */
    std::optional<User> getUserById(int user_id);

    /**
     * Update an existing user's profile information
     * Note: This does NOT update username, email, or password
     * @param user_id ID of user to update
     * @param name Optional new full name
     * @param position Optional new position
     * @param phone_number Optional new phone number
     * @param university Optional new university
     * @param department Optional new department
     * @param enrollment_year Optional new enrollment year
     * @param primary_language Optional new primary language
     * @return true if update successful, false otherwise
     */
    bool updateUser(
        int user_id,
        const std::optional<std::string>& name = std::nullopt,
        const std::optional<std::string>& position = std::nullopt,
        const std::optional<std::string>& phone_number = std::nullopt,
        const std::optional<std::string>& university = std::nullopt,
        const std::optional<std::string>& department = std::nullopt,
        const std::optional<int>& enrollment_year = std::nullopt,
        const std::optional<std::string>& primary_language = std::nullopt
    );

    /**
     * Authenticate a user by username and password
     * This function retrieves the user and verifies the password using bcrypt
     * @param username Username to authenticate
     * @param password Plain text password to verify
     * @return User object if authentication successful, nullopt otherwise
     */
    std::optional<User> authenticateUser(
        const std::string& username,
        const std::string& password
    );

    /**
     * Get the last error message from a failed operation
     * @return Error message string
     */
    const std::string& getLastError() const { return last_error_; }

private:
    std::shared_ptr<repositories::UserRepository> user_repository_;
    std::string last_error_;

    void setError(const std::string& error) { last_error_ = error; }
    void clearError() { last_error_.clear(); }
};

} // namespace helpers
} // namespace sohbet
