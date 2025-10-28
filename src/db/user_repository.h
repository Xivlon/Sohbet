#pragma once

#include <optional>
#include <string>

namespace sohbet {
namespace db {

struct User {
    long long id = -1;
    std::string id_no;            // id_no (optional, unique)
    std::string username;
    std::string name;
    std::string email;
    std::string clearance_level;
    std::string institution;
    std::string specialization;
    int graduation_year = 0;      // 0 = unknown
    std::string phone;
    std::string role;
    int account_warnings = 0;
    std::string created_at;
};

class Database; // forward

// Repository functions for higher-level user operations
class UserRepository {
public:
    explicit UserRepository(Database& db);

    // Create a new user. The password is plaintext and will be hashed.
    // Returns the new row id on success, or -1 on error.
    long long createUser(
        const std::string& id_no,
        const std::string& username,
        const std::string& name,
        const std::string& email,
        const std::string& plaintextPassword,
        const std::string& clearance_level = "",
        const std::string& institution = "",
        const std::string& specialization = "",
        int graduation_year = 0,
        const std::string& phone = "",
        const std::string& role = "user",
        int account_warnings = 0,
        int bcryptRounds = 12
    );

    // Find a user by username. Returns std::nullopt if not found.
    std::optional<User> findByUsername(const std::string& username);

    // Authenticate a user: returns true if password matches.
    bool authenticate(const std::string& username, const std::string& plaintextPassword);

private:
    Database& db_;
};

} // namespace db
} // namespace sohbet
