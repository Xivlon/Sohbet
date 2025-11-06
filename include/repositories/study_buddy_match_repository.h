#pragma once

#include "db/database.h"
#include "models/study_buddy_match.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

/**
 * Repository for StudyBuddyMatch data operations
 */
class StudyBuddyMatchRepository {
public:
    /**
     * Constructor
     * @param database Database instance
     */
    explicit StudyBuddyMatchRepository(std::shared_ptr<db::Database> database);

    /**
     * Create a new study buddy match
     * @param match StudyBuddyMatch object
     * @return StudyBuddyMatch with ID if successful, nullopt otherwise
     */
    std::optional<StudyBuddyMatch> create(const StudyBuddyMatch& match);

    /**
     * Update an existing study buddy match
     * @param match StudyBuddyMatch object with updated values
     * @return true if successful, false otherwise
     */
    bool update(const StudyBuddyMatch& match);

    /**
     * Find match by ID
     * @param matchId Match ID
     * @return StudyBuddyMatch if found, nullopt otherwise
     */
    std::optional<StudyBuddyMatch> findById(int matchId);

    /**
     * Find all matches for a user
     * @param userId User ID
     * @param status Optional status filter
     * @param limit Maximum number of matches to return
     * @return Vector of StudyBuddyMatch objects
     */
    std::vector<StudyBuddyMatch> findByUserId(
        int userId,
        std::optional<MatchStatus> status = std::nullopt,
        int limit = 50
    );

    /**
     * Find suggested matches for a user, ordered by compatibility score
     * @param userId User ID
     * @param limit Maximum number of matches to return
     * @return Vector of suggested StudyBuddyMatch objects
     */
    std::vector<StudyBuddyMatch> findSuggestedMatches(int userId, int limit = 20);

    /**
     * Find accepted matches (confirmed study buddies)
     * @param userId User ID
     * @return Vector of accepted StudyBuddyMatch objects
     */
    std::vector<StudyBuddyMatch> findAcceptedMatches(int userId);

    /**
     * Check if a match exists between two users
     * @param userId1 First user ID
     * @param userId2 Second user ID
     * @return StudyBuddyMatch if exists, nullopt otherwise
     */
    std::optional<StudyBuddyMatch> findMatchBetweenUsers(int userId1, int userId2);

    /**
     * Update match status
     * @param matchId Match ID
     * @param status New status
     * @return true if successful, false otherwise
     */
    bool updateStatus(int matchId, MatchStatus status);

    /**
     * Mark match as viewed
     * @param matchId Match ID
     * @return true if successful, false otherwise
     */
    bool markAsViewed(int matchId);

    /**
     * Delete a match
     * @param matchId Match ID
     * @return true if successful, false otherwise
     */
    bool deleteById(int matchId);

    /**
     * Delete all matches for a user
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deleteByUserId(int userId);

private:
    std::shared_ptr<db::Database> database_;

    /**
     * Helper to build StudyBuddyMatch from database row
     */
    StudyBuddyMatch buildFromRow(sqlite3_stmt* stmt);
};

} // namespace repositories
} // namespace sohbet
