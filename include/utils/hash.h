#pragma once
#include <string>

namespace sohbet {
namespace utils {

// =======================
// SECURE BCRYPT HASHING
// =======================

// Hash a password using bcrypt (secure)
std::string hash_password(const std::string& password);

// Verify a password against stored bcrypt hash (secure)
bool verify_password(const std::string& password, const std::string& stored_hash);

// Legacy functions (for backward compatibility)
std::string generate_salt_legacy();
std::string hash_password_legacy(const std::string& password);
bool verify_password_legacy(const std::string& password, const std::string& stored_hash);

// =======================
// LEGACY / PLACEHOLDER HASHING (INSECURE)
// =======================

/**
 * SECURITY WARNING: This is a placeholder hashing implementation using std::hash + salt.
 * This is INSECURE and MUST be replaced with bcrypt or Argon2 before any production deployment.
 * This exists only for backward compatibility / development purposes.
 */
class Hash {
public:
    // Generate a salt for password hashing
    static std::string generateSalt();
    
    // Hash a password with a salt (INSECURE PLACEHOLDER)
    static std::string hashPassword(const std::string& password, const std::string& salt);
    
    // Verify a password against a hash (INSECURE PLACEHOLDER)
    static bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    
    // Generate a salted hash from a password (convenience method)
    static std::string generateSaltedHash(const std::string& password);
    
    // Verify a password against a salted hash (convenience method)
    static bool verifySaltedHash(const std::string& password, const std::string& salted_hash);

private:
    static std::string generateRandomString(size_t length);
};

} // namespace utils
} // namespace sohbet