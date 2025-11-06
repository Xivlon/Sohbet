#include "models/study_buddy_match.h"
#include <algorithm>

StudyBuddyMatch::StudyBuddyMatch()
    : id(0),
      user_id(0),
      matched_user_id(0),
      compatibility_score(0.0),
      course_overlap_score(0.0),
      schedule_compatibility_score(0.0),
      learning_style_score(0.0),
      academic_level_score(0.0),
      status(MatchStatus::SUGGESTED),
      viewed_at(0),
      responded_at(0),
      created_at(std::time(nullptr)),
      updated_at(std::time(nullptr)) {}

json StudyBuddyMatch::toJson() const {
    json coursesArray = json::array();
    for (const auto& course : common_courses) {
        coursesArray.push_back(course);
    }

    json interestsArray = json::array();
    for (const auto& interest : common_interests) {
        interestsArray.push_back(interest);
    }

    json result = {
        {"id", id},
        {"user_id", user_id},
        {"matched_user_id", matched_user_id},
        {"compatibility_score", compatibility_score},
        {"course_overlap_score", course_overlap_score},
        {"schedule_compatibility_score", schedule_compatibility_score},
        {"learning_style_score", learning_style_score},
        {"academic_level_score", academic_level_score},
        {"common_courses", coursesArray},
        {"common_interests", interestsArray},
        {"match_reason", match_reason},
        {"status", matchStatusToString(status)},
        {"created_at", created_at},
        {"updated_at", updated_at}
    };

    if (viewed_at > 0) {
        result["viewed_at"] = viewed_at;
    }
    if (responded_at > 0) {
        result["responded_at"] = responded_at;
    }

    return result;
}

StudyBuddyMatch StudyBuddyMatch::fromJson(const json& j) {
    StudyBuddyMatch match;
    match.id = j.value("id", 0);
    match.user_id = j.value("user_id", 0);
    match.matched_user_id = j.value("matched_user_id", 0);
    match.compatibility_score = j.value("compatibility_score", 0.0);
    match.course_overlap_score = j.value("course_overlap_score", 0.0);
    match.schedule_compatibility_score = j.value("schedule_compatibility_score", 0.0);
    match.learning_style_score = j.value("learning_style_score", 0.0);
    match.academic_level_score = j.value("academic_level_score", 0.0);

    if (j.contains("common_courses") && j["common_courses"].is_array()) {
        for (const auto& course : j["common_courses"]) {
            match.common_courses.push_back(course.get<std::string>());
        }
    }

    if (j.contains("common_interests") && j["common_interests"].is_array()) {
        for (const auto& interest : j["common_interests"]) {
            match.common_interests.push_back(interest.get<std::string>());
        }
    }

    match.match_reason = j.value("match_reason", "");
    match.status = stringToMatchStatus(j.value("status", "suggested"));
    match.viewed_at = j.value("viewed_at", 0);
    match.responded_at = j.value("responded_at", 0);
    match.created_at = j.value("created_at", std::time(nullptr));
    match.updated_at = j.value("updated_at", std::time(nullptr));

    return match;
}

std::string StudyBuddyMatch::matchStatusToString(MatchStatus status) {
    switch (status) {
        case MatchStatus::SUGGESTED: return "suggested";
        case MatchStatus::ACCEPTED: return "accepted";
        case MatchStatus::DECLINED: return "declined";
        case MatchStatus::BLOCKED: return "blocked";
        default: return "suggested";
    }
}

MatchStatus StudyBuddyMatch::stringToMatchStatus(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "accepted") return MatchStatus::ACCEPTED;
    if (lower == "declined") return MatchStatus::DECLINED;
    if (lower == "blocked") return MatchStatus::BLOCKED;
    return MatchStatus::SUGGESTED;
}

void StudyBuddyMatch::calculateCompatibilityScore(
    double courseWeight,
    double scheduleWeight,
    double learningStyleWeight,
    double academicLevelWeight
) {
    compatibility_score =
        (course_overlap_score * courseWeight) +
        (schedule_compatibility_score * scheduleWeight) +
        (learning_style_score * learningStyleWeight) +
        (academic_level_score * academicLevelWeight);

    // Ensure score is in 0-100 range
    compatibility_score = std::max(0.0, std::min(100.0, compatibility_score));
}
