#pragma once

#include <string>

namespace sohbet {
namespace db {

// Simple bcrypt wrapper: requires a bcrypt C library exposing
// bcrypt_gensalt(int rounds, char *salt) and bcrypt_hashpw(const char *key, const char *salt).
// Many libbcrypt variants also let you pass the stored hash as the "salt" to re-compute a check hash.
class Bcrypt {
public:
    // rounds: work factor (commonly 10-14). Default 12.
    // Throws std::runtime_error on error.
    static std::string hashPassword(const std::string& password, int rounds = 12);

    // Verify plaintext password against stored bcrypt hash.
    // Returns true if password matches, false otherwise.
    // Throws std::runtime_error on unexpected errors.
    static bool verifyPassword(const std::string& password, const std::string& storedHash);
};

} // namespace db
} // namespace sohbet
