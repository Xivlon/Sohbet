#include "security/jwt.h"
#include <array>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace sohbet {
namespace security {

// Base64 URL encoding (RFC 4648)
std::string base64_url_encode(const std::string& input) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    result.reserve(((input.size() + 2) / 3) * 4);

    int val = 0;
    int valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    return result;
}

// Base64 URL decoding (RFC 4648)
std::string base64_url_decode(const std::string& input) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::array<int, 256> lookup{};
    lookup.fill(-1);
    for (int i = 0; i < 64; ++i) {
        lookup[static_cast<unsigned char>(chars[i])] = i;
    }

    std::string result;
    result.reserve((input.size() * 3) / 4);

    int val = 0;
    int valb = -8;
    for (unsigned char c : input) {
        if (c == '=') {
            break; // Padding character (not expected for URL-safe encoding)
        }
        int decoded = lookup[c];
        if (decoded == -1) {
            return ""; // Invalid character encountered
        }
        val = (val << 6) + decoded;
        valb += 6;
        if (valb >= 0) {
            result.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return result;
}

// HMAC SHA256 (simplified, requires OpenSSL)
std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char* digest = HMAC(EVP_sha256(), key.c_str(), key.length(),
                                reinterpret_cast<const unsigned char*>(data.c_str()),
                                data.length(), nullptr, nullptr);
    return std::string(reinterpret_cast<char*>(digest), 32);
}

std::string generate_jwt_token(const std::string& username, int user_id, const std::string& role, const std::string& secret, int expiry_hours) {
    // JWT Header
    std::string header = R"({"alg":"HS256","typ":"JWT"})";
    
    // JWT Payload with expiration
    auto now = std::chrono::system_clock::now();
    auto exp_time = now + std::chrono::hours(expiry_hours);
    auto exp_timestamp = std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
    
    std::ostringstream payload_ss;
    payload_ss << R"({"username":")" << username << R"(",)";
    payload_ss << R"("user_id":)" << user_id << R"(,)";
    payload_ss << R"("role":")" << role << R"(",)";
    payload_ss << R"("exp":)" << exp_timestamp << "}";
    std::string payload = payload_ss.str();
    
    // Encode header and payload
    std::string encoded_header = base64_url_encode(header);
    std::string encoded_payload = base64_url_encode(payload);
    
    // Create signature
    std::string to_sign = encoded_header + "." + encoded_payload;
    std::string signature = hmac_sha256(secret, to_sign);
    std::string encoded_signature = base64_url_encode(signature);
    
    return encoded_header + "." + encoded_payload + "." + encoded_signature;
}

std::optional<JWTPayload> verify_jwt_token(const std::string& token, const std::string& secret) {
    // Split token into parts
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);
    
    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        return std::nullopt;
    }
    
    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string encoded_signature = token.substr(second_dot + 1);
    
    // Verify signature
    std::string to_verify = encoded_header + "." + encoded_payload;
    std::string expected_signature = hmac_sha256(secret, to_verify);
    std::string expected_encoded = base64_url_encode(expected_signature);
    
    if (encoded_signature != expected_encoded) {
        return std::nullopt; // Invalid signature
    }
    
    // Decode payload
    std::string payload_json = base64_url_decode(encoded_payload);
    
    // Check if decoding was successful and has minimum JSON structure
    if (payload_json.length() < 2 || payload_json[0] != '{') {
        return std::nullopt; // Invalid base64 encoding or not a JSON object
    }
    
    // Parse payload (simplified JSON parsing)
    JWTPayload payload;
    
    // Extract username
    size_t username_pos = payload_json.find("\"username\":\"");
    if (username_pos == std::string::npos) {
        return std::nullopt; // Missing username field
    }
    size_t username_start = username_pos + 12;
    size_t username_end = payload_json.find("\"", username_start);
    if (username_end == std::string::npos) {
        return std::nullopt; // Malformed username field
    }
    payload.username = payload_json.substr(username_start, username_end - username_start);
    
    // Extract user_id
    size_t user_id_pos = payload_json.find("\"user_id\":");
    if (user_id_pos == std::string::npos) {
        return std::nullopt; // Missing user_id field
    }
    size_t user_id_start = user_id_pos + 10;
    // Find either comma or closing brace as delimiter
    size_t user_id_end = payload_json.find(",", user_id_start);
    size_t user_id_end_alt = payload_json.find("}", user_id_start);
    if (user_id_end == std::string::npos) {
        user_id_end = user_id_end_alt;
    } else if (user_id_end_alt != std::string::npos && user_id_end_alt < user_id_end) {
        user_id_end = user_id_end_alt;
    }
    if (user_id_end == std::string::npos) {
        return std::nullopt; // Malformed user_id field
    }
    try {
        payload.user_id = std::stoi(payload_json.substr(user_id_start, user_id_end - user_id_start));
    } catch (const std::exception&) {
        return std::nullopt; // Invalid user_id format
    }
    
    // Extract role
    size_t role_pos = payload_json.find("\"role\":\"");
    if (role_pos != std::string::npos) {
        size_t role_start = role_pos + 8;
        size_t role_end = payload_json.find("\"", role_start);
        if (role_end != std::string::npos) {
            payload.role = payload_json.substr(role_start, role_end - role_start);
        } else {
            payload.role = "Student"; // Default role
        }
    } else {
        payload.role = "Student"; // Default role
    }
    
    // Extract expiration
    size_t exp_pos = payload_json.find("\"exp\":");
    if (exp_pos == std::string::npos) {
        return std::nullopt; // Missing expiration field
    }
    size_t exp_start = exp_pos + 6;
    // Find either comma or closing brace as delimiter
    size_t exp_end = payload_json.find(",", exp_start);
    size_t exp_end_alt = payload_json.find("}", exp_start);
    if (exp_end == std::string::npos) {
        exp_end = exp_end_alt;
    } else if (exp_end_alt != std::string::npos && exp_end_alt < exp_end) {
        exp_end = exp_end_alt;
    }
    if (exp_end == std::string::npos) {
        return std::nullopt; // Malformed expiration field
    }
    try {
        payload.exp = std::stoll(payload_json.substr(exp_start, exp_end - exp_start));
    } catch (const std::exception&) {
        return std::nullopt; // Invalid expiration format
    }
    
    // Check expiration
    auto now = std::chrono::system_clock::now();
    auto now_timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    if (payload.exp < now_timestamp) {
        return std::nullopt; // Token expired
    }
    
    return payload;
}

} // namespace security
} // namespace sohbet
