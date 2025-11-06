#pragma once

#include "repositories/study_preferences_repository.h"
#include "repositories/study_buddy_match_repository.h"
#include "repositories/study_buddy_connection_repository.h"
#include "repositories/user_repository.h"
#include "models/study_buddy_match.h"
#include "models/user.h"
#include <memory>
#include <vector>

namespace sohbet {
namespace services {

/**
 * Service for intelligent study buddy matching algorithm
 */
class StudyBuddyMatchingService {
public:
    /**
     * Constructor
     */
    StudyBuddyMatchingService(
        std::shared_ptr<repositories::StudyPreferencesRepository> prefsRepo,
        std::shared_ptr<repositories::StudyBuddyMatchRepository> matchRepo,
        std::shared_ptr<repositories::UserRepository> userRepo
    );

    /**
     * Generate study buddy matches for a user
     * @param userId User ID to generate matches for
     * @param limit Maximum number of matches to generate
     * @return Vector of StudyBuddyMatch objects
     */
    std::vector<StudyBuddyMatch> generateMatches(int userId, int limit = 20);

    /**
     * Calculate compatibility score between two users
     * @param user1Id First user ID
     * @param user2Id Second user ID
     * @return StudyBuddyMatch with calculated scores, or nullopt if calculation fails
     */
    std::optional<StudyBuddyMatch> calculateCompatibility(int user1Id, int user2Id);

    /**
     * Refresh matches for a user (regenerate based on current preferences)
     * @param userId User ID
     * @return Number of matches generated
     */
    int refreshMatches(int userId);

    /**
     * Get recommended study buddies for a user
     * @param userId User ID
     * @param limit Maximum number of recommendations
     * @return Vector of StudyBuddyMatch objects with user details
     */
    std::vector<StudyBuddyMatch> getRecommendations(int userId, int limit = 20);

private:
    std::shared_ptr<repositories::StudyPreferencesRepository> prefsRepo_;
    std::shared_ptr<repositories::StudyBuddyMatchRepository> matchRepo_;
    std::shared_ptr<repositories::UserRepository> userRepo_;

    /**
     * Calculate course overlap score (0-100)
     */
    double calculateCourseOverlapScore(
        const StudyPreferences& prefs1,
        const StudyPreferences& prefs2
    );

    /**
     * Calculate schedule compatibility score (0-100)
     */
    double calculateScheduleCompatibilityScore(
        const StudyPreferences& prefs1,
        const StudyPreferences& prefs2
    );

    /**
     * Calculate learning style compatibility score (0-100)
     */
    double calculateLearningStyleScore(
        const StudyPreferences& prefs1,
        const StudyPreferences& prefs2
    );

    /**
     * Calculate academic level score (0-100)
     */
    double calculateAcademicLevelScore(
        const User& user1,
        const User& user2,
        const StudyPreferences& prefs1,
        const StudyPreferences& prefs2
    );

    /**
     * Find common elements in two vectors
     */
    std::vector<std::string> findCommonElements(
        const std::vector<std::string>& vec1,
        const std::vector<std::string>& vec2
    );

    /**
     * Generate match reason text
     */
    std::string generateMatchReason(const StudyBuddyMatch& match);
};

} // namespace services
} // namespace sohbet
