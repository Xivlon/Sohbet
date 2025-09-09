#pragma once
#include "models/user.h"
#include "db/database.h"
#include <memory>
#include <optional>

namespace sohbet {

class UserRepository {
public:
    explicit UserRepository(Database& db);
    
    // Initialize/migrate database schema
    bool migrate();
    
    // Create a new user
    std::optional<User> create(const User& user, const std::string& password);
    
    // Find user by username
    std::optional<User> findByUsername(const std::string& username);
    
    // Find user by email
    std::optional<User> findByEmail(const std::string& email);
    
    // Check if username exists
    bool usernameExists(const std::string& username);
    
    // Check if email exists
    bool emailExists(const std::string& email);

private:
    Database& db_;
    User userFromStatement(sqlite3_stmt* stmt);
};

} // namespace sohbet