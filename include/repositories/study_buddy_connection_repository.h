#pragma once

#include "db/database.h"
#include "models/study_buddy_connection.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

/**
 * Repository for StudyBuddyConnection data operations
 */
class StudyBuddyConnectionRepository {
public:
    /**
     * Constructor
     * @param database Database instance
     */
    explicit StudyBuddyConnectionRepository(std::shared_ptr<db::Database> database);

    /**
     * Create a new study buddy connection
     * @param connection StudyBuddyConnection object
     * @return StudyBuddyConnection with ID if successful, nullopt otherwise
     */
    std::optional<StudyBuddyConnection> create(const StudyBuddyConnection& connection);

    /**
     * Update an existing connection
     * @param connection StudyBuddyConnection object with updated values
     * @return true if successful, false otherwise
     */
    bool update(const StudyBuddyConnection& connection);

    /**
     * Find connection by ID
     * @param connectionId Connection ID
     * @return StudyBuddyConnection if found, nullopt otherwise
     */
    std::optional<StudyBuddyConnection> findById(int connectionId);

    /**
     * Find all connections for a user
     * @param userId User ID
     * @return Vector of StudyBuddyConnection objects
     */
    std::vector<StudyBuddyConnection> findByUserId(int userId);

    /**
     * Find connection between two users
     * @param userId1 First user ID
     * @param userId2 Second user ID
     * @return StudyBuddyConnection if found, nullopt otherwise
     */
    std::optional<StudyBuddyConnection> findConnectionBetweenUsers(int userId1, int userId2);

    /**
     * Find favorite study buddies
     * @param userId User ID
     * @return Vector of favorite StudyBuddyConnection objects
     */
    std::vector<StudyBuddyConnection> findFavorites(int userId);

    /**
     * Get strongest connections (highest connection strength)
     * @param userId User ID
     * @param limit Maximum number to return
     * @return Vector of StudyBuddyConnection objects
     */
    std::vector<StudyBuddyConnection> getStrongestConnections(int userId, int limit = 10);

    /**
     * Update connection strength for a connection
     * @param connectionId Connection ID
     * @return true if successful, false otherwise
     */
    bool recalculateConnectionStrength(int connectionId);

    /**
     * Increment study session count and update last session time
     * @param connectionId Connection ID
     * @return true if successful, false otherwise
     */
    bool incrementStudySessions(int connectionId);

    /**
     * Delete a connection
     * @param connectionId Connection ID
     * @return true if successful, false otherwise
     */
    bool deleteById(int connectionId);

    /**
     * Delete connection between two users
     * @param userId1 First user ID
     * @param userId2 Second user ID
     * @return true if successful, false otherwise
     */
    bool deleteConnectionBetweenUsers(int userId1, int userId2);

private:
    std::shared_ptr<db::Database> database_;

    /**
     * Helper to build StudyBuddyConnection from database row
     */
    StudyBuddyConnection buildFromRow(db::Statement& stmt);
};

} // namespace repositories
} // namespace sohbet
