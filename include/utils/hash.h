#pragma once
#include <string>

namespace sohbet {
namespace utils {

// PLACEHOLDER HASHING - TO BE REPLACED WITH BCRYPT
// WARNING: This is NOT secure and should NOT be used in production

// Generate a salt for hashing
std::string generate_salt();

// Hash a password with salt (INSECURE PLACEHOLDER)
std::string hash_password(const std::string& password);

// Verify a password against stored hash (INSECURE PLACEHOLDER)
bool verify_password(const std::string& password, const std::string& stored_hash);

} // namespace utils
} // namespace sohbet