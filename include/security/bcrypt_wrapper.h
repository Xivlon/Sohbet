#pragma once
#include <string>

namespace sohbet {
namespace security {

// Secure bcrypt-based password hashing
std::string hash_password_bcrypt(const std::string& password, int rounds = 12);

// Verify a password against a bcrypt hash
bool verify_password_bcrypt(const std::string& password, const std::string& hash);

} // namespace security
} // namespace sohbet