#include "db/password.h"
#include <stdexcept>
#include <cstring>
#include <cstdlib>

// This file expects a libbcrypt-like C API.
// Example functions (libbcrypt):
//   int bcrypt_gensalt(int workfactor, char *salt_out);
//   char* bcrypt_hashpw(const char *key, const char *salt);
// If you use a different bcrypt library, adapt calls accordingly.

extern "C" {
    // declare expected C functions from the bcrypt library
    // If your chosen library has different names, update these or adapt in a shim.
    int bcrypt_gensalt(int, char*);
    char* bcrypt_hashpw(const char*, const char*);
}

namespace sohbet {
namespace db {

std::string Bcrypt::hashPassword(const std::string& password, int rounds) {
    if (password.empty()) throw std::runtime_error("password is empty");

    // salt buffer: typical libbcrypt uses 64 bytes for salt/hash output
    char saltbuf[128];
    std::memset(saltbuf, 0, sizeof(saltbuf));

    int rc = bcrypt_gensalt(rounds, saltbuf);
    if (rc != 0) {
        throw std::runtime_error("bcrypt_gensalt failed");
    }

    char* hashed = bcrypt_hashpw(password.c_str(), saltbuf);
    if (!hashed) {
        throw std::runtime_error("bcrypt_hashpw returned null");
    }

    std::string result(hashed);
    // many implementations allocate with malloc; free to avoid leak
    std::free(hashed);
    return result;
}

bool Bcrypt::verifyPassword(const std::string& password, const std::string& storedHash) {
    if (password.empty()) return false;
    if (storedHash.empty()) return false;

    // Some bcrypt libraries let you pass the stored hash as the salt argument
    // to bcrypt_hashpw to produce a candidate hash to compare against.
    char* candidate = bcrypt_hashpw(password.c_str(), storedHash.c_str());
    if (!candidate) {
        throw std::runtime_error("bcrypt_hashpw (verify) returned null");
    }
    std::string candStr(candidate);
    std::free(candidate);

    return candStr == storedHash;
}

} // namespace db
} // namespace sohbet