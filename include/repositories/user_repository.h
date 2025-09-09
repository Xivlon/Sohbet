#pragma once

#include "db/database.h"
#include "models/user.h"
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

/**
 * Repository for User data operations
 */
class UserRepository {
public:
    /**
     * Constructor
     * @param database Database instance
     */
    explicit UserRepository(std::shared_ptr<db::Database> database);
    
    /**
     * Run database migrations to create the users table
     * @return true if successful, false otherwise
     */
    bool migrate();
    
    /**
     * Create a new user with password hashing
     * @param user User object to create (ID will be set after creation)
     * @param password Plaintext password
     * @return User object if successful, nullopt otherwise
     */
    std::optional<User> create(User& user, const std::string& password);
    
    /**
     * Find a user by username
     * @param username Username to search for
     * @return User object if found, nullopt otherwise
     */
    std::optional<User> findByUsername(const std::string& username);
    
    /**
     * Find a user by email
     * @param email Email to search for
     * @return User object if found, nullopt otherwise
     */
    std::optional<User> findByEmail(const std::string& email);
    
    /**
     * Find a user by ID
     * @param id User ID to search for
     * @return User object if found, nullopt otherwise
     */
    std::optional<User> findById(int id);
    
    /**
     * Check if username exists
     * @param username Username to check
     * @return true if exists, false otherwise
     */
    bool usernameExists(const std::string& username);
    
    /**
     * Check if email exists
     * @param email Email to check
     * @return true if exists, false otherwise
     */
    bool emailExists(const std::string& email);

private:
    std::shared_ptr<db::Database> database_;
    
    /**
     * Helper method to build User object from database row
     * @param stmt Prepared statement positioned at a result row
     * @return User object
     */
    User userFromStatement(db::Statement& stmt);
};

} // namespace repositories
} // namespace sohbet