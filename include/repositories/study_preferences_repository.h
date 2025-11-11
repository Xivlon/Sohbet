#pragma once

#include "db/database.h"
#include "models/study_preferences.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

/**
 * Repository for StudyPreferences data operations
 */
class StudyPreferencesRepository {
public:
    /**
     * Constructor
     * @param database Database instance
     */
    explicit StudyPreferencesRepository(std::shared_ptr<db::Database> database);

    /**
     * Create or update study preferences for a user
     * @param prefs StudyPreferences object
     * @return StudyPreferences with ID if successful, nullopt otherwise
     */
    std::optional<StudyPreferences> upsert(const StudyPreferences& prefs);

    /**
     * Find study preferences by user ID
     * @param userId User ID to search for
     * @return StudyPreferences if found, nullopt otherwise
     */
    std::optional<StudyPreferences> findByUserId(int userId);

    /**
     * Find all active study preferences
     * @return Vector of active StudyPreferences
     */
    std::vector<StudyPreferences> findAllActive();

    /**
     * Find study preferences by learning style
     * @param learningStyle Learning style to filter by
     * @return Vector of matching StudyPreferences
     */
    std::vector<StudyPreferences> findByLearningStyle(LearningStyle learningStyle);

    /**
     * Find study preferences with course overlap
     * @param courses Vector of courses to match
     * @return Vector of StudyPreferences with overlapping courses
     */
    std::vector<StudyPreferences> findByCourses(const std::vector<std::string>& courses);

    /**
     * Deactivate study preferences for a user
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deactivate(int userId);

    /**
     * Delete study preferences for a user
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deleteByUserId(int userId);

private:
    std::shared_ptr<db::Database> database_;

    /**
     * Helper to build StudyPreferences from database row
     */
    StudyPreferences buildFromRow(db::Statement& stmt);
};

} // namespace repositories
} // namespace sohbet
