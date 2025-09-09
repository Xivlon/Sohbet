#include "utils/hash.h"
#include <functional>
#include <random>
#include <iomanip>
#include <sstream>

namespace sohbet {
namespace utils {

// PLACEHOLDER IMPLEMENTATION - NOT SECURE
// This will be replaced with bcrypt

std::string generate_salt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << dis(gen);
    }
    return oss.str();
}

std::string hash_password(const std::string& password) {
    // INSECURE PLACEHOLDER: std::hash with fixed salt
    // This will be replaced with bcrypt
    std::string salt = generate_salt();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + salt);
    
    std::ostringstream oss;
    oss << salt << ":" << hash_value;
    return oss.str();
}

bool verify_password(const std::string& password, const std::string& stored_hash) {
    // INSECURE PLACEHOLDER: extract salt and verify
    // This will be replaced with bcrypt verification
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