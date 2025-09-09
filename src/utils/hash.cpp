#include "utils/hash.h"
#include "security/bcrypt_wrapper.h"
#include <functional>
#include <random>
#include <iomanip>
#include <sstream>

namespace sohbet {
namespace utils {

// Main API functions - now using bcrypt
std::string hash_password(const std::string& password) {
    return security::hash_password_bcrypt(password);
}

bool verify_password(const std::string& password, const std::string& stored_hash) {
    // First try bcrypt verification
    if (stored_hash.length() >= 60 && stored_hash[0] == '$') {
        // Looks like a bcrypt hash
        return security::verify_password_bcrypt(password, stored_hash);
    }
    
    // Fall back to legacy verification for existing data
    return verify_password_legacy(password, stored_hash);
}

// Legacy implementation for backward compatibility (DEPRECATED)
std::string generate_salt_legacy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << dis(gen);
    }
    return oss.str();
}

std::string hash_password_legacy(const std::string& password) {
    // INSECURE PLACEHOLDER: std::hash with fixed salt
    std::string salt = generate_salt_legacy();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + salt);
    
    std::ostringstream oss;
    oss << salt << ":" << hash_value;
    return oss.str();
}

bool verify_password_legacy(const std::string& password, const std::string& stored_hash) {
    // INSECURE PLACEHOLDER: extract salt and verify
    size_t separator = stored_hash.find(':');
    if (separator == std::string::npos) {
        return false;
    }
    
    std::string salt = stored_hash.substr(0, separator);
    std::string hash_part = stored_hash.substr(separator + 1);
    
    std::hash<std::string> hasher;
    size_t computed_hash = hasher(password + salt);
    
    return std::to_string(computed_hash) == hash_part;
}

} // namespace utils
} // namespace sohbet