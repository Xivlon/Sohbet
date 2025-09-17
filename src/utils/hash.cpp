#include "utils/hash.h"
#include "security/bcrypt_wrapper.h"
#include <functional>
#include <random>
#include <iomanip>
#include <sstream>

namespace sohbet {
namespace utils {

// -------------------- Modern bcrypt API --------------------
std::string hash_password(const std::string& password) {
    return security::hash_password_bcrypt(password);
}

bool verify_password(const std::string& password, const std::string& stored_hash) {
    // If it looks like a bcrypt hash
    if (!stored_hash.empty() && stored_hash[0] == '$' && stored_hash.length() >= 60) {
        return security::verify_password_bcrypt(password, stored_hash);
    }
    // Fallback to legacy verification
    return verify_password_legacy(password, stored_hash);
}

// -------------------- Legacy (insecure) hashing --------------------
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
    std::string salt = generate_salt_legacy();
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + salt);

    std::ostringstream oss;
    oss << salt << ":" << hash_value;
    return oss.str();
}

bool verify_password_legacy(const std::string& password, const std::string& stored_hash) {
    size_t separator = stored_hash.find(':');
    if (separator == std::string::npos) return false;

    std::string salt = stored_hash.substr(0, separator);
    std::string hash_part = stored_hash.substr(separator + 1);

    std::hash<std::string> hasher;
    size_t computed_hash = hasher(password + salt);

    return std::to_string(computed_hash) == hash_part;
}

// Hash class implementation for backward compatibility
std::string Hash::generateSalt() {
    return generate_salt_legacy();
}

std::string Hash::hashPassword(const std::string& password, const std::string& salt) {
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + salt);
    return std::to_string(hash_value);
}

bool Hash::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::hash<std::string> hasher;
    size_t computed_hash = hasher(password + salt);
    return std::to_string(computed_hash) == hash;
}

std::string Hash::generateSaltedHash(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);
    return salt + ":" + hash;
}

bool Hash::verifySaltedHash(const std::string& password, const std::string& salted_hash) {
    size_t separator = salted_hash.find(':');
    if (separator == std::string::npos) return false;
    
    std::string salt = salted_hash.substr(0, separator);
    std::string hash = salted_hash.substr(separator + 1);
    
    return verifyPassword(password, hash, salt);
}

std::string Hash::generateRandomString(size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(2) << dis(gen);
    }
    return oss.str();
}

} // namespace utils
} // namespace sohbet