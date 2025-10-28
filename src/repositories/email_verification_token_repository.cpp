#include "repositories/email_verification_token_repository.h"
#include <iostream>
#include <ctime>

namespace sohbet {
namespace repositories {

EmailVerificationTokenRepository::EmailVerificationTokenRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<EmailVerificationToken> EmailVerificationTokenRepository::createToken(int user_id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    std::string token = EmailVerificationToken::generateToken();
    std::time_t expires_at = EmailVerificationToken::getDefaultExpiration();
    
    const std::string sql = "INSERT INTO email_verification_tokens (user_id, token, expires_at) VALUES (?, ?, ?)";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for creating verification token" << std::endl;
        return std::nullopt;
    }
    
    stmt.bindInt(1, user_id);
    stmt.bindText(2, token);
    stmt.bindInt(3, static_cast<int>(expires_at));
    
    int result = stmt.step();
    
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to insert verification token" << std::endl;
        return std::nullopt;
    }
    
    // Create and return the token object
    EmailVerificationToken verification_token(user_id, token, expires_at);
    verification_token.setId(static_cast<int>(database_->lastInsertRowId()));
    
    return verification_token;
}

std::optional<EmailVerificationToken> EmailVerificationTokenRepository::findByToken(const std::string& token) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, token, expires_at, created_at, verified_at
        FROM email_verification_tokens WHERE token = ? LIMIT 1
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for finding token" << std::endl;
        return std::nullopt;
    }
    
    stmt.bindText(1, token);
    
    if (stmt.step() == SQLITE_ROW) {
        return buildTokenFromStatement(stmt);
    }
    
    return std::nullopt;
}

bool EmailVerificationTokenRepository::verifyToken(const std::string& token) {
    if (!database_ || !database_->isOpen()) return false;

    // First, check if token exists and is valid
    auto token_obj = findByToken(token);
    if (!token_obj.has_value()) {
        return false;
    }
    
    // Check if already verified
    if (token_obj->isVerified()) {
        return true; // Already verified
    }
    
    // Check if expired
    if (token_obj->isExpired()) {
        return false;
    }
    
    // Mark token as verified
    const std::string sql = "UPDATE email_verification_tokens SET verified_at = CURRENT_TIMESTAMP WHERE token = ?";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for verifying token" << std::endl;
        return false;
    }
    
    stmt.bindText(1, token);
    
    if (stmt.step() != SQLITE_DONE) {
        std::cerr << "Failed to verify token" << std::endl;
        return false;
    }
    
    // Also update the user's email_verified field
    const std::string update_user_sql = "UPDATE users SET email_verified = 1 WHERE id = ?";
    db::Statement user_stmt(*database_, update_user_sql);
    if (!user_stmt.isValid()) {
        std::cerr << "Failed to prepare statement for updating user email_verified" << std::endl;
        return false;
    }
    
    user_stmt.bindInt(1, token_obj->getUserId());
    
    return user_stmt.step() == SQLITE_DONE;
}

int EmailVerificationTokenRepository::deleteExpiredTokens() {
    if (!database_ || !database_->isOpen()) return 0;

    std::time_t now = std::time(nullptr);
    const std::string sql = "DELETE FROM email_verification_tokens WHERE expires_at < ?";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for deleting expired tokens" << std::endl;
        return 0;
    }
    
    stmt.bindInt(1, static_cast<int>(now));
    
    if (stmt.step() != SQLITE_DONE) {
        std::cerr << "Failed to delete expired tokens" << std::endl;
        return 0;
    }
    
    // Since we can't get the exact count easily with the current API, return 1 for success
    return 1;
}

std::optional<EmailVerificationToken> EmailVerificationTokenRepository::findLatestByUserId(int user_id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, token, expires_at, created_at, verified_at
        FROM email_verification_tokens WHERE user_id = ?
        ORDER BY created_at DESC LIMIT 1
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for finding latest token" << std::endl;
        return std::nullopt;
    }
    
    stmt.bindInt(1, user_id);
    
    if (stmt.step() == SQLITE_ROW) {
        return buildTokenFromStatement(stmt);
    }
    
    return std::nullopt;
}

bool EmailVerificationTokenRepository::deleteTokensForUser(int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM email_verification_tokens WHERE user_id = ?";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare statement for deleting user tokens" << std::endl;
        return false;
    }
    
    stmt.bindInt(1, user_id);
    
    return stmt.step() == SQLITE_DONE;
}

EmailVerificationToken EmailVerificationTokenRepository::buildTokenFromStatement(db::Statement& stmt) {
    EmailVerificationToken token;
    
    // id (column 0)
    token.setId(stmt.getInt(0));
    
    // user_id (column 1)
    token.setUserId(stmt.getInt(1));
    
    // token (column 2)
    token.setToken(stmt.getText(2));
    
    // expires_at (column 3)
    token.setExpiresAt(static_cast<std::time_t>(stmt.getInt(3)));
    
    // created_at (column 4)
    std::string created_at = stmt.getText(4);
    if (!created_at.empty()) {
        token.setCreatedAt(created_at);
    }
    
    // verified_at (column 5)
    std::string verified_at = stmt.getText(5);
    if (!verified_at.empty()) {
        token.setVerifiedAt(verified_at);
    }
    
    return token;
}

} // namespace repositories
} // namespace sohbet
