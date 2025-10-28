#pragma once

#include <string>
#include <optional>
#include <ctime>

namespace sohbet {

/**
 * EmailVerificationToken model
 * Represents a token for verifying user email addresses
 */
class EmailVerificationToken {
public:
    EmailVerificationToken() = default;
    EmailVerificationToken(int user_id, const std::string& token, std::time_t expires_at);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getUserId() const { return user_id_; }
    const std::string& getToken() const { return token_; }
    std::time_t getExpiresAt() const { return expires_at_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getVerifiedAt() const { return verified_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setToken(const std::string& token) { token_ = token; }
    void setExpiresAt(std::time_t expires_at) { expires_at_ = expires_at; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setVerifiedAt(const std::optional<std::string>& verified_at) { verified_at_ = verified_at; }

    // Utility methods
    bool isExpired() const;
    bool isVerified() const { return verified_at_.has_value(); }

    // Static helper methods
    static std::string generateToken();
    static std::time_t getDefaultExpiration(); // 24 hours from now

    // JSON serialization
    std::string toJson() const;

private:
    std::optional<int> id_;
    int user_id_;
    std::string token_;
    std::time_t expires_at_;
    std::optional<std::string> created_at_;
    std::optional<std::string> verified_at_;
};

} // namespace sohbet
