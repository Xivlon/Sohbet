#pragma once
#include <string>
#include <optional>

namespace sohbet {
namespace security {

// Simple JWT token utilities (for educational/demo purposes)
// In production, use a proper JWT library

struct JWTPayload {
    std::string username;
    int user_id;
    std::string role;
    long long exp; // expiration timestamp
};

// Generate a JWT token for a user
std::string generate_jwt_token(const std::string& username, int user_id, const std::string& role = "Student", const std::string& secret = "default_secret", int expiry_hours = 24);

// Verify and decode a JWT token
std::optional<JWTPayload> verify_jwt_token(const std::string& token, const std::string& secret = "default_secret");

} // namespace security
} // namespace sohbet