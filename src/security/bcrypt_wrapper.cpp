#include "security/bcrypt_wrapper.h"
#include <bcrypt.h>
#include <stdexcept>
#include <cstring>

namespace sohbet {
namespace security {

std::string hash_password_bcrypt(const std::string& password, int rounds) {
    char salt[BCRYPT_HASHSIZE];
    char hash[BCRYPT_HASHSIZE];
    
    // Generate salt with specified rounds
    if (bcrypt_gensalt(rounds, salt) != 0) {
        throw std::runtime_error("Failed to generate salt");
    }
    
    // Hash the password
    if (bcrypt_hashpw(password.c_str(), salt, hash) != 0) {
        throw std::runtime_error("Failed to hash password");
    }
    
    return std::string(hash);
}

bool verify_password_bcrypt(const std::string& password, const std::string& hash) {
    // bcrypt_checkpw returns 0 on success, -1 on failure
    return bcrypt_checkpw(password.c_str(), hash.c_str()) == 0;
}

} // namespace security
} // namespace sohbet