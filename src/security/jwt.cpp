#include "security/jwt.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace sohbet {
namespace security {

// Base64 URL encoding (simplified)
std::string base64_url_encode(const std::string& input) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    int val = 0, valb = -6;
    
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    
    return result;
}

// Base64 URL decoding (simplified)
std::string base64_url_decode(const std::string& input) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    int val = 0, valb = -8;
    
    for (char c : input) {
        if (c == '=') break;
        val = (val << 6) + chars.find(c);
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
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

std::string generate_jwt_token(const std::string& username, int user_id, const std::string& secret, int expiry_hours) {
    // JWT Header
    std::string header = R"({"alg":"HS256","typ":"JWT"})";
    
    // JWT Payload with expiration
    auto now = std::chrono::system_clock::now();
    auto exp_time = now + std::chrono::hours(expiry_hours);
    auto exp_timestamp = std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
    
    std::ostringstream payload_ss;
    payload_ss << R"({"username":")" << username << R"(",)";
    payload_ss << R"("user_id":)" << user_id << R"(,)";
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
    
    // Parse payload (simplified JSON parsing)
    JWTPayload payload;
    
    // Extract username
    size_t username_start = payload_json.find("\"username\":\"") + 12;
    size_t username_end = payload_json.find("\"", username_start);
    payload.username = payload_json.substr(username_start, username_end - username_start);
    
    // Extract user_id
    size_t user_id_start = payload_json.find("\"user_id\":") + 10;
    size_t user_id_end = payload_json.find(",", user_id_start);
    payload.user_id = std::stoi(payload_json.substr(user_id_start, user_id_end - user_id_start));
    
    // Extract expiration
    size_t exp_start = payload_json.find("\"exp\":") + 6;
    size_t exp_end = payload_json.find("}", exp_start);
    payload.exp = std::stoll(payload_json.substr(exp_start, exp_end - exp_start));
    
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