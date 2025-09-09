#pragma once
#include <string>

namespace sohbet {
namespace utils {

// Secure bcrypt-based password hashing 
// This replaces the previous INSECURE placeholder implementation

// Hash a password using bcrypt (secure)
std::string hash_password(const std::string& password);

// Verify a password against stored bcrypt hash (secure)
bool verify_password(const std::string& password, const std::string& stored_hash);

// Legacy functions for backward compatibility (DEPRECATED - will be removed)
std::string hash_password_legacy(const std::string& password);
bool verify_password_legacy(const std::string& password, const std::string& stored_hash);

} // namespace utils
} // namespace sohbet