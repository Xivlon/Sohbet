#pragma once

#include <string>

namespace sohbet {
namespace utils {

/**
 * SECURITY WARNING: This is a placeholder hashing implementation using std::hash + salt.
 * This is INSECURE and MUST be replaced with bcrypt or Argon2 before any production deployment.
 * 
 * This implementation is only for development/prototype purposes.
 */
class Hash {
public:
    /**
     * Generate a salt for password hashing
     * @return A random salt string
     */
    static std::string generateSalt();
    
    /**
     * Hash a password with a salt (INSECURE PLACEHOLDER)
     * @param password The plain text password
     * @param salt The salt to use
     * @return The hashed password
     */
    static std::string hashPassword(const std::string& password, const std::string& salt);
    
    /**
     * Verify a password against a hash (INSECURE PLACEHOLDER)
     * @param password The plain text password to verify
     * @param hash The stored hash to verify against
     * @param salt The salt used for hashing
     * @return true if password matches, false otherwise
     */
    static bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    
    /**
     * Generate a salted hash from a password (convenience method)
     * @param password The plain text password
     * @return A string containing salt:hash
     */
    static std::string generateSaltedHash(const std::string& password);
    
    /**
     * Verify a password against a salted hash (convenience method)
     * @param password The plain text password
     * @param salted_hash The stored salted hash (salt:hash format)
     * @return true if password matches, false otherwise
     */
    static bool verifySaltedHash(const std::string& password, const std::string& salted_hash);

private:
    static std::string generateRandomString(size_t length);
};

} // namespace utils
} // namespace sohbet