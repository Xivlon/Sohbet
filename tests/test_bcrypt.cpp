#include "utils/hash.h"
#include <iostream>
#include <cassert>

void test_bcrypt_hashing() {
    std::string password = "TestPassword123";
    
    // Hash the password
    std::string hash = sohbet::utils::hash_password(password);
    std::cout << "Generated bcrypt hash: " << hash << std::endl;
    
    // Verify the hash should be a bcrypt format (starts with $2 and is ~60 chars)
    assert(hash.length() >= 59);
    assert(hash[0] == '$');
    assert(hash[1] == '2');
    
    // Verify correct password
    assert(sohbet::utils::verify_password(password, hash));
    
    // Verify incorrect password fails
    assert(!sohbet::utils::verify_password("WrongPassword", hash));
    
    std::cout << "Bcrypt password hashing test passed!" << std::endl;
}

void test_legacy_compatibility() {
    // Test that we can still verify old-style hashes for backward compatibility
    std::string legacy_hash = sohbet::utils::hash_password_legacy("OldPassword123");
    std::cout << "Legacy hash format: " << legacy_hash << std::endl;
    
    // Should be able to verify legacy format
    assert(sohbet::utils::verify_password("OldPassword123", legacy_hash));
    assert(!sohbet::utils::verify_password("WrongPassword", legacy_hash));
    
    std::cout << "Legacy compatibility test passed!" << std::endl;
}

void test_different_passwords_different_hashes() {
    std::string hash1 = sohbet::utils::hash_password("Password1");
    std::string hash2 = sohbet::utils::hash_password("Password1");
    std::string hash3 = sohbet::utils::hash_password("Password2");
    
    // Same password should produce different hashes (due to salt)
    assert(hash1 != hash2);
    
    // Different passwords should produce different hashes
    assert(hash1 != hash3);
    assert(hash2 != hash3);
    
    // But all should verify correctly
    assert(sohbet::utils::verify_password("Password1", hash1));
    assert(sohbet::utils::verify_password("Password1", hash2));
    assert(sohbet::utils::verify_password("Password2", hash3));
    
    std::cout << "Different passwords produce different hashes test passed!" << std::endl;
}

int main() {
    try {
        test_bcrypt_hashing();
        test_legacy_compatibility();
        test_different_passwords_different_hashes();
        std::cout << "All bcrypt tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}