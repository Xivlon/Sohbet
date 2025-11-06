#include "services/study_buddy_matching_service.h"
#include <algorithm>
#include <set>
#include <sstream>
#include <cmath>

namespace sohbet {
namespace services {

StudyBuddyMatchingService::StudyBuddyMatchingService(
    std::shared_ptr<repositories::StudyPreferencesRepository> prefsRepo,
    std::shared_ptr<repositories::StudyBuddyMatchRepository> matchRepo,
    std::shared_ptr<repositories::UserRepository> userRepo
) : prefsRepo_(prefsRepo), matchRepo_(matchRepo), userRepo_(userRepo) {}

std::vector<StudyBuddyMatch> StudyBuddyMatchingService::generateMatches(int userId, int limit) {
    std::vector<StudyBuddyMatch> matches;

    // Get user's preferences
    auto userPrefs = prefsRepo_->findByUserId(userId);
    if (!userPrefs) return matches;

    auto user = userRepo_->findById(userId);
    if (!user) return matches;

    // Get all active preferences (potential matches)
    auto allPrefs = prefsRepo_->findAllActive();

    // Calculate compatibility with each potential match
    std::vector<StudyBuddyMatch> potentialMatches;

    for (const auto& candidatePrefs : allPrefs) {
        // Skip self
        if (candidatePrefs.user_id == userId) continue;

        // Get candidate user info
        auto candidate = userRepo_->findById(candidatePrefs.user_id);
        if (!candidate) continue;

        // Apply preference filters
        if (userPrefs->same_university_only) {
            if (!user->getUniversity() || !candidate->getUniversity() ||
                user->getUniversity().value() != candidate->getUniversity().value()) {
                continue;
            }
        }

        if (userPrefs->same_department_only) {
            if (!user->getDepartment() || !candidate->getDepartment() ||
                user->getDepartment().value() != candidate->getDepartment().value()) {
                continue;
            }
        }

        if (userPrefs->same_year_only) {
            if (!user->getEnrollmentYear() || !candidate->getEnrollmentYear() ||
                user->getEnrollmentYear().value() != candidate->getEnrollmentYear().value()) {
                continue;
            }
        }

        // Calculate compatibility
        StudyBuddyMatch match;
        match.user_id = userId;
        match.matched_user_id = candidatePrefs.user_id;

        // Calculate individual scores
        match.course_overlap_score = calculateCourseOverlapScore(*userPrefs, candidatePrefs);
        match.schedule_compatibility_score = calculateScheduleCompatibilityScore(*userPrefs, candidatePrefs);
        match.learning_style_score = calculateLearningStyleScore(*userPrefs, candidatePrefs);
        match.academic_level_score = calculateAcademicLevelScore(*user, *candidate, *userPrefs, candidatePrefs);

        // Calculate overall compatibility
        match.calculateCompatibilityScore();

        // Find common courses and interests
        match.common_courses = findCommonElements(userPrefs->courses, candidatePrefs.courses);
        match.common_interests = findCommonElements(userPrefs->topics_of_interest, candidatePrefs.topics_of_interest);

        // Generate match reason
        match.match_reason = generateMatchReason(match);

        // Only include if compatibility is above threshold (e.g., 40%)
        if (match.compatibility_score >= 40.0) {
            potentialMatches.push_back(match);
        }
    }

    // Sort by compatibility score (highest first)
    std::sort(potentialMatches.begin(), potentialMatches.end(),
              [](const StudyBuddyMatch& a, const StudyBuddyMatch& b) {
                  return a.compatibility_score > b.compatibility_score;
              });

    // Limit results
    int count = std::min(limit, static_cast<int>(potentialMatches.size()));
    for (int i = 0; i < count; ++i) {
        matches.push_back(potentialMatches[i]);
    }

    return matches;
}

std::optional<StudyBuddyMatch> StudyBuddyMatchingService::calculateCompatibility(int user1Id, int user2Id) {
    auto prefs1 = prefsRepo_->findByUserId(user1Id);
    auto prefs2 = prefsRepo_->findByUserId(user2Id);

    if (!prefs1 || !prefs2) return std::nullopt;

    auto user1 = userRepo_->findById(user1Id);
    auto user2 = userRepo_->findById(user2Id);

    if (!user1 || !user2) return std::nullopt;

    StudyBuddyMatch match;
    match.user_id = user1Id;
    match.matched_user_id = user2Id;

    match.course_overlap_score = calculateCourseOverlapScore(*prefs1, *prefs2);
    match.schedule_compatibility_score = calculateScheduleCompatibilityScore(*prefs1, *prefs2);
    match.learning_style_score = calculateLearningStyleScore(*prefs1, *prefs2);
    match.academic_level_score = calculateAcademicLevelScore(*user1, *user2, *prefs1, *prefs2);

    match.calculateCompatibilityScore();

    match.common_courses = findCommonElements(prefs1->courses, prefs2->courses);
    match.common_interests = findCommonElements(prefs1->topics_of_interest, prefs2->topics_of_interest);
    match.match_reason = generateMatchReason(match);

    return match;
}

int StudyBuddyMatchingService::refreshMatches(int userId) {
    // Delete old suggested matches
    auto existingMatches = matchRepo_->findByUserId(userId, MatchStatus::SUGGESTED);
    for (const auto& match : existingMatches) {
        matchRepo_->deleteById(match.id);
    }

    // Generate new matches
    auto newMatches = generateMatches(userId);

    // Save new matches to database
    int savedCount = 0;
    for (const auto& match : newMatches) {
        if (matchRepo_->create(match)) {
            savedCount++;
        }
    }

    return savedCount;
}

std::vector<StudyBuddyMatch> StudyBuddyMatchingService::getRecommendations(int userId, int limit) {
    // Try to get existing suggested matches from database
    auto existingMatches = matchRepo_->findSuggestedMatches(userId, limit);

    // If we have enough matches, return them
    if (static_cast<int>(existingMatches.size()) >= limit) {
        return existingMatches;
    }

    // Otherwise, generate fresh matches
    return generateMatches(userId, limit);
}

double StudyBuddyMatchingService::calculateCourseOverlapScore(
    const StudyPreferences& prefs1,
    const StudyPreferences& prefs2
) {
    if (prefs1.courses.empty() && prefs2.courses.empty()) {
        return 50.0; // Neutral score if both have no courses
    }

    if (prefs1.courses.empty() || prefs2.courses.empty()) {
        return 20.0; // Low score if one has no courses
    }

    auto common = findCommonElements(prefs1.courses, prefs2.courses);

    // Calculate Jaccard similarity
    std::set<std::string> union_set(prefs1.courses.begin(), prefs1.courses.end());
    union_set.insert(prefs2.courses.begin(), prefs2.courses.end());

    double jaccard = static_cast<double>(common.size()) / static_cast<double>(union_set.size());

    // Also consider topic overlap
    auto commonTopics = findCommonElements(prefs1.topics_of_interest, prefs2.topics_of_interest);
    double topicBonus = std::min(20.0, commonTopics.size() * 5.0);

    return std::min(100.0, (jaccard * 80.0) + topicBonus);
}

double StudyBuddyMatchingService::calculateScheduleCompatibilityScore(
    const StudyPreferences& prefs1,
    const StudyPreferences& prefs2
) {
    double score = 0.0;

    // Check time preference compatibility
    if (prefs1.study_time_preference == prefs2.study_time_preference) {
        score += 40.0;
    } else if (prefs1.study_time_preference == StudyTimePreference::FLEXIBLE ||
               prefs2.study_time_preference == StudyTimePreference::FLEXIBLE) {
        score += 30.0;
    } else {
        score += 10.0; // Different preferences but not incompatible
    }

    // Check environment compatibility
    if (prefs1.study_environment == prefs2.study_environment) {
        score += 30.0;
    } else if (prefs1.study_environment == StudyEnvironment::FLEXIBLE ||
               prefs2.study_environment == StudyEnvironment::FLEXIBLE) {
        score += 20.0;
    } else {
        score += 5.0;
    }

    // Check available days overlap
    auto commonDays = findCommonElements(prefs1.available_days, prefs2.available_days);
    double dayScore = std::min(30.0, commonDays.size() * 6.0);
    score += dayScore;

    return std::min(100.0, score);
}

double StudyBuddyMatchingService::calculateLearningStyleScore(
    const StudyPreferences& prefs1,
    const StudyPreferences& prefs2
) {
    // Perfect match: same style or one is mixed
    if (prefs1.learning_style == prefs2.learning_style) {
        return 100.0;
    }

    if (prefs1.learning_style == LearningStyle::MIXED ||
        prefs2.learning_style == LearningStyle::MIXED) {
        return 80.0;
    }

    // Complementary styles can work well together
    // Visual + Reading/Writing = good
    if ((prefs1.learning_style == LearningStyle::VISUAL && prefs2.learning_style == LearningStyle::READING_WRITING) ||
        (prefs1.learning_style == LearningStyle::READING_WRITING && prefs2.learning_style == LearningStyle::VISUAL)) {
        return 70.0;
    }

    // Auditory + Kinesthetic = moderate
    if ((prefs1.learning_style == LearningStyle::AUDITORY && prefs2.learning_style == LearningStyle::KINESTHETIC) ||
        (prefs1.learning_style == LearningStyle::KINESTHETIC && prefs2.learning_style == LearningStyle::AUDITORY)) {
        return 60.0;
    }

    // Other combinations
    return 50.0;
}

double StudyBuddyMatchingService::calculateAcademicLevelScore(
    const User& user1,
    const User& user2,
    const StudyPreferences& prefs1,
    const StudyPreferences& prefs2
) {
    double score = 50.0; // Base score

    // Same university bonus
    if (user1.getUniversity() && user2.getUniversity() &&
        user1.getUniversity().value() == user2.getUniversity().value()) {
        score += 20.0;
    }

    // Same department bonus
    if (user1.getDepartment() && user2.getDepartment() &&
        user1.getDepartment().value() == user2.getDepartment().value()) {
        score += 15.0;
    }

    // Enrollment year proximity (closer years = better)
    if (user1.getEnrollmentYear() && user2.getEnrollmentYear()) {
        int yearDiff = std::abs(user1.getEnrollmentYear().value() - user2.getEnrollmentYear().value());
        if (yearDiff == 0) {
            score += 15.0; // Same year
        } else if (yearDiff == 1) {
            score += 10.0; // One year apart
        } else if (yearDiff == 2) {
            score += 5.0; // Two years apart
        }
    }

    return std::min(100.0, score);
}

std::vector<std::string> StudyBuddyMatchingService::findCommonElements(
    const std::vector<std::string>& vec1,
    const std::vector<std::string>& vec2
) {
    std::vector<std::string> common;
    std::set<std::string> set2(vec2.begin(), vec2.end());

    for (const auto& item : vec1) {
        if (set2.find(item) != set2.end()) {
            common.push_back(item);
        }
    }

    return common;
}

std::string StudyBuddyMatchingService::generateMatchReason(const StudyBuddyMatch& match) {
    std::ostringstream reason;

    if (!match.common_courses.empty()) {
        if (match.common_courses.size() == 1) {
            reason << "Taking " << match.common_courses[0] << " together";
        } else {
            reason << "Taking " << match.common_courses.size() << " courses together";
        }
    } else if (!match.common_interests.empty()) {
        reason << "Shared interest in " << match.common_interests[0];
    } else if (match.learning_style_score >= 80.0) {
        reason << "Compatible learning styles";
    } else if (match.schedule_compatibility_score >= 80.0) {
        reason << "Similar study schedules";
    } else if (match.academic_level_score >= 80.0) {
        reason << "Same academic program";
    } else {
        reason << "Good overall compatibility";
    }

    return reason.str();
}

} // namespace services
} // namespace sohbet
