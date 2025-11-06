#include "models/study_preferences.h"
#include <algorithm>

StudyPreferences::StudyPreferences()
    : id(0),
      user_id(0),
      learning_style(LearningStyle::MIXED),
      study_environment(StudyEnvironment::FLEXIBLE),
      study_time_preference(StudyTimePreference::FLEXIBLE),
      available_hours_per_week(5),
      preferred_group_size(2),
      same_university_only(true),
      same_department_only(false),
      same_year_only(false),
      is_active(true),
      created_at(std::time(nullptr)),
      updated_at(std::time(nullptr)) {}

json StudyPreferences::toJson() const {
    json coursesArray = json::array();
    for (const auto& course : courses) {
        coursesArray.push_back(course);
    }

    json topicsArray = json::array();
    for (const auto& topic : topics_of_interest) {
        topicsArray.push_back(topic);
    }

    json daysArray = json::array();
    for (const auto& day : available_days) {
        daysArray.push_back(day);
    }

    return json{
        {"id", id},
        {"user_id", user_id},
        {"learning_style", learningStyleToString(learning_style)},
        {"study_environment", studyEnvironmentToString(study_environment)},
        {"study_time_preference", studyTimePreferenceToString(study_time_preference)},
        {"courses", coursesArray},
        {"topics_of_interest", topicsArray},
        {"academic_goals", academic_goals},
        {"available_days", daysArray},
        {"available_hours_per_week", available_hours_per_week},
        {"preferred_group_size", preferred_group_size},
        {"same_university_only", same_university_only},
        {"same_department_only", same_department_only},
        {"same_year_only", same_year_only},
        {"is_active", is_active},
        {"created_at", created_at},
        {"updated_at", updated_at}
    };
}

StudyPreferences StudyPreferences::fromJson(const json& j) {
    StudyPreferences prefs;
    prefs.id = j.value("id", 0);
    prefs.user_id = j.value("user_id", 0);

    prefs.learning_style = stringToLearningStyle(j.value("learning_style", "mixed"));
    prefs.study_environment = stringToStudyEnvironment(j.value("study_environment", "flexible"));
    prefs.study_time_preference = stringToStudyTimePreference(j.value("study_time_preference", "flexible"));

    if (j.contains("courses") && j["courses"].is_array()) {
        for (const auto& course : j["courses"]) {
            prefs.courses.push_back(course.get<std::string>());
        }
    }

    if (j.contains("topics_of_interest") && j["topics_of_interest"].is_array()) {
        for (const auto& topic : j["topics_of_interest"]) {
            prefs.topics_of_interest.push_back(topic.get<std::string>());
        }
    }

    if (j.contains("available_days") && j["available_days"].is_array()) {
        for (const auto& day : j["available_days"]) {
            prefs.available_days.push_back(day.get<std::string>());
        }
    }

    prefs.academic_goals = j.value("academic_goals", "");
    prefs.available_hours_per_week = j.value("available_hours_per_week", 5);
    prefs.preferred_group_size = j.value("preferred_group_size", 2);
    prefs.same_university_only = j.value("same_university_only", true);
    prefs.same_department_only = j.value("same_department_only", false);
    prefs.same_year_only = j.value("same_year_only", false);
    prefs.is_active = j.value("is_active", true);
    prefs.created_at = j.value("created_at", std::time(nullptr));
    prefs.updated_at = j.value("updated_at", std::time(nullptr));

    return prefs;
}

std::string StudyPreferences::learningStyleToString(LearningStyle style) {
    switch (style) {
        case LearningStyle::VISUAL: return "visual";
        case LearningStyle::AUDITORY: return "auditory";
        case LearningStyle::READING_WRITING: return "reading_writing";
        case LearningStyle::KINESTHETIC: return "kinesthetic";
        case LearningStyle::MIXED: return "mixed";
        default: return "mixed";
    }
}

LearningStyle StudyPreferences::stringToLearningStyle(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "visual") return LearningStyle::VISUAL;
    if (lower == "auditory") return LearningStyle::AUDITORY;
    if (lower == "reading_writing") return LearningStyle::READING_WRITING;
    if (lower == "kinesthetic") return LearningStyle::KINESTHETIC;
    return LearningStyle::MIXED;
}

std::string StudyPreferences::studyEnvironmentToString(StudyEnvironment env) {
    switch (env) {
        case StudyEnvironment::QUIET: return "quiet";
        case StudyEnvironment::MODERATE: return "moderate";
        case StudyEnvironment::LIVELY: return "lively";
        case StudyEnvironment::FLEXIBLE: return "flexible";
        default: return "flexible";
    }
}

StudyEnvironment StudyPreferences::stringToStudyEnvironment(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "quiet") return StudyEnvironment::QUIET;
    if (lower == "moderate") return StudyEnvironment::MODERATE;
    if (lower == "lively") return StudyEnvironment::LIVELY;
    return StudyEnvironment::FLEXIBLE;
}

std::string StudyPreferences::studyTimePreferenceToString(StudyTimePreference pref) {
    switch (pref) {
        case StudyTimePreference::EARLY_MORNING: return "early_morning";
        case StudyTimePreference::MORNING: return "morning";
        case StudyTimePreference::AFTERNOON: return "afternoon";
        case StudyTimePreference::EVENING: return "evening";
        case StudyTimePreference::NIGHT: return "night";
        case StudyTimePreference::FLEXIBLE: return "flexible";
        default: return "flexible";
    }
}

StudyTimePreference StudyPreferences::stringToStudyTimePreference(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "early_morning") return StudyTimePreference::EARLY_MORNING;
    if (lower == "morning") return StudyTimePreference::MORNING;
    if (lower == "afternoon") return StudyTimePreference::AFTERNOON;
    if (lower == "evening") return StudyTimePreference::EVENING;
    if (lower == "night") return StudyTimePreference::NIGHT;
    return StudyTimePreference::FLEXIBLE;
}
