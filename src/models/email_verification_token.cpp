#include "models/email_verification_token.h"
#include <sstream>
#include <random>
#include <iomanip>
#include <ctime>

namespace sohbet {

EmailVerificationToken::EmailVerificationToken(int user_id, const std::string& token, std::time_t expires_at)
    : user_id_(user_id), token_(token), expires_at_(expires_at) {}

bool EmailVerificationToken::isExpired() const {
    std::time_t now = std::time(nullptr);
    return now > expires_at_;
}

std::string EmailVerificationToken::generateToken() {
    // Generate a secure random token (64 characters hex)
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    std::stringstream ss;
    for (int i = 0; i < 4; ++i) {
        uint64_t rand_val = dis(gen);
        ss << std::hex << std::setfill('0') << std::setw(16) << rand_val;
    }
    
    return ss.str();
}

std::time_t EmailVerificationToken::getDefaultExpiration() {
    // 24 hours from now
    return std::time(nullptr) + (24 * 60 * 60);
}

std::string EmailVerificationToken::toJson() const {
    std::stringstream ss;
    ss << "{";
    
    if (id_.has_value()) {
        ss << "\"id\":" << id_.value() << ",";
    }
    
    ss << "\"user_id\":" << user_id_ << ","
       << "\"token\":\"" << token_ << "\",";
    
    // Convert time_t to ISO 8601 string
    char expires_buf[32];
    std::tm* expires_tm = std::gmtime(&expires_at_);
    std::strftime(expires_buf, sizeof(expires_buf), "%Y-%m-%dT%H:%M:%SZ", expires_tm);
    ss << "\"expires_at\":\"" << expires_buf << "\"";
    
    if (created_at_.has_value()) {
        ss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }
    
    if (verified_at_.has_value()) {
        ss << ",\"verified_at\":\"" << verified_at_.value() << "\"";
    }
    
    ss << "}";
    return ss.str();
}

} // namespace sohbet
