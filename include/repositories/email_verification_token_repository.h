#pragma once

#include "models/email_verification_token.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <string>

namespace sohbet {
namespace repositories {

/**
 * Repository for managing email verification tokens in the database
 */
class EmailVerificationTokenRepository {
public:
    explicit EmailVerificationTokenRepository(std::shared_ptr<db::Database> database);

    /**
     * Create a new verification token for a user
     * @param user_id The user ID to create a token for
     * @return The created token with ID, or nullopt on failure
     */
    std::optional<EmailVerificationToken> createToken(int user_id);

    /**
     * Find a token by its token string
     * @param token The token string to search for
     * @return The token if found and valid, nullopt otherwise
     */
    std::optional<EmailVerificationToken> findByToken(const std::string& token);

    /**
     * Mark a token as verified
     * @param token The token string to verify
     * @return true if successful, false otherwise
     */
    bool verifyToken(const std::string& token);

    /**
     * Delete expired tokens (cleanup)
     * @return Number of deleted tokens
     */
    int deleteExpiredTokens();

    /**
     * Get the latest token for a user
     * @param user_id The user ID
     * @return The latest token if found, nullopt otherwise
     */
    std::optional<EmailVerificationToken> findLatestByUserId(int user_id);

    /**
     * Delete all tokens for a user
     * @param user_id The user ID
     * @return true if successful, false otherwise
     */
    bool deleteTokensForUser(int user_id);

private:
    std::shared_ptr<db::Database> database_;

    /**
     * Helper to build EmailVerificationToken from DB statement
     */
    EmailVerificationToken buildTokenFromStatement(db::Statement& stmt);
};

} // namespace repositories
} // namespace sohbet
