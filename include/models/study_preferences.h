#ifndef STUDY_PREFERENCES_H
#define STUDY_PREFERENCES_H

#include <string>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class LearningStyle {
    VISUAL,
    AUDITORY,
    READING_WRITING,
    KINESTHETIC,
    MIXED
};

enum class StudyEnvironment {
    QUIET,
    MODERATE,
    LIVELY,
    FLEXIBLE
};

enum class StudyTimePreference {
    EARLY_MORNING,  // 5-8am
    MORNING,        // 8-12pm
    AFTERNOON,      // 12-5pm
    EVENING,        // 5-9pm
    NIGHT,          // 9pm-1am
    FLEXIBLE
};

class StudyPreferences {
public:
    int id;
    int user_id;

    // Learning preferences
    LearningStyle learning_style;
    StudyEnvironment study_environment;
    StudyTimePreference study_time_preference;

    // Academic focus
    std::vector<std::string> courses;
    std::vector<std::string> topics_of_interest;
    std::string academic_goals;

    // Availability
    std::vector<std::string> available_days;
    int available_hours_per_week;

    // Matching preferences
    int preferred_group_size;
    bool same_university_only;
    bool same_department_only;
    bool same_year_only;

    // Metadata
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;

    StudyPreferences();

    // Conversion methods
    json toJson() const;
    static StudyPreferences fromJson(const json& j);

    // Helper methods
    static std::string learningStyleToString(LearningStyle style);
    static LearningStyle stringToLearningStyle(const std::string& str);

    static std::string studyEnvironmentToString(StudyEnvironment env);
    static StudyEnvironment stringToStudyEnvironment(const std::string& str);

    static std::string studyTimePreferenceToString(StudyTimePreference pref);
    static StudyTimePreference stringToStudyTimePreference(const std::string& str);
};

#endif // STUDY_PREFERENCES_H
