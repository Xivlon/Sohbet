#include "security/bcrypt_wrapper.h"
#include <bcrypt/BCrypt.hpp>

namespace sohbet {
namespace security {

std::string hash_password_bcrypt(const std::string& password, int rounds) {
    return BCrypt::generateHash(password, rounds);
}

bool verify_password_bcrypt(const std::string& password, const std::string& hash) {
    return BCrypt::validatePassword(password, hash);
}

} // namespace security
} // namespace sohbet