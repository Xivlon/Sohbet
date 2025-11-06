#ifndef STUDY_BUDDY_MATCH_H
#define STUDY_BUDDY_MATCH_H

#include <string>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class MatchStatus {
    SUGGESTED,
    ACCEPTED,
    DECLINED,
    BLOCKED
};

class StudyBuddyMatch {
public:
    int id;
    int user_id;
    int matched_user_id;

    // Compatibility scoring (0-100)
    double compatibility_score;
    double course_overlap_score;
    double schedule_compatibility_score;
    double learning_style_score;
    double academic_level_score;

    // Match metadata
    std::vector<std::string> common_courses;
    std::vector<std::string> common_interests;
    std::string match_reason;

    // Match status
    MatchStatus status;
    std::time_t viewed_at;
    std::time_t responded_at;

    // Metadata
    std::time_t created_at;
    std::time_t updated_at;

    StudyBuddyMatch();

    // Conversion methods
    json toJson() const;
    static StudyBuddyMatch fromJson(const json& j);

    // Helper methods
    static std::string matchStatusToString(MatchStatus status);
    static MatchStatus stringToMatchStatus(const std::string& str);

    // Compatibility calculation
    void calculateCompatibilityScore(
        double courseWeight = 0.35,
        double scheduleWeight = 0.25,
        double learningStyleWeight = 0.25,
        double academicLevelWeight = 0.15
    );
};

#endif // STUDY_BUDDY_MATCH_H
