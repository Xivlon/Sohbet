#include "utils/hash.h"
#include <functional>
#include <random>
#include <iomanip>
#include <sstream>

namespace sohbet {
namespace utils {

std::string Hash::generateSalt() {
    return generateRandomString(16);
}

std::string Hash::hashPassword(const std::string& password, const std::string& salt) {
    // WARNING: This is an INSECURE placeholder implementation!
    // Using std::hash is NOT suitable for password hashing in production.
    // Replace with bcrypt, Argon2, or similar before production deployment.
    
    std::string salted_password = salt + password + salt;
    std::hash<std::string> hasher;
    size_t hash_value = hasher(salted_password);
    
    // Convert to hex string
    std::ostringstream oss;
    oss << std::hex << hash_value;
    return oss.str();
}

bool Hash::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string computed_hash = hashPassword(password, salt);
    return computed_hash == hash;
}

std::string Hash::generateSaltedHash(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);
    return salt + ":" + hash;
}

bool Hash::verifySaltedHash(const std::string& password, const std::string& salted_hash) {
    size_t separator_pos = salted_hash.find(':');
    if (separator_pos == std::string::npos) {
        return false;
    }
    
    std::string salt = salted_hash.substr(0, separator_pos);
    std::string hash = salted_hash.substr(separator_pos + 1);
    
    return verifyPassword(password, hash, salt);
}

std::string Hash::generateRandomString(size_t length) {
    const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    
    return result;
}

} // namespace utils
} // namespace sohbet