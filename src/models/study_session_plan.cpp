#include "models/study_session_plan.h"
#include <algorithm>

StudySessionPlan::StudySessionPlan()
    : id(0),
      creator_id(0),
      session_type(SessionType::PAIR),
      scheduled_start(0),
      scheduled_end(0),
      max_participants(5),
      current_participants(1),
      status(SessionStatus::PLANNED),
      completion_rating(0),
      created_at(std::time(nullptr)),
      updated_at(std::time(nullptr)) {}

json StudySessionPlan::toJson() const {
    json result = {
        {"id", id},
        {"creator_id", creator_id},
        {"title", title},
        {"description", description},
        {"course_topic", course_topic},
        {"session_type", sessionTypeToString(session_type)},
        {"scheduled_start", scheduled_start},
        {"scheduled_end", scheduled_end},
        {"location", location},
        {"meeting_link", meeting_link},
        {"max_participants", max_participants},
        {"current_participants", current_participants},
        {"status", sessionStatusToString(status)},
        {"created_at", created_at},
        {"updated_at", updated_at}
    };

    if (completion_rating > 0) {
        result["completion_rating"] = completion_rating;
    }
    if (!completion_notes.empty()) {
        result["completion_notes"] = completion_notes;
    }

    return result;
}

StudySessionPlan StudySessionPlan::fromJson(const json& j) {
    StudySessionPlan plan;
    plan.id = j.value("id", 0);
    plan.creator_id = j.value("creator_id", 0);
    plan.title = j.value("title", "");
    plan.description = j.value("description", "");
    plan.course_topic = j.value("course_topic", "");
    plan.session_type = stringToSessionType(j.value("session_type", "pair"));
    plan.scheduled_start = j.value("scheduled_start", 0);
    plan.scheduled_end = j.value("scheduled_end", 0);
    plan.location = j.value("location", "");
    plan.meeting_link = j.value("meeting_link", "");
    plan.max_participants = j.value("max_participants", 5);
    plan.current_participants = j.value("current_participants", 1);
    plan.status = stringToSessionStatus(j.value("status", "planned"));
    plan.completion_rating = j.value("completion_rating", 0);
    plan.completion_notes = j.value("completion_notes", "");
    plan.created_at = j.value("created_at", std::time(nullptr));
    plan.updated_at = j.value("updated_at", std::time(nullptr));

    return plan;
}

std::string StudySessionPlan::sessionTypeToString(SessionType type) {
    switch (type) {
        case SessionType::INDIVIDUAL: return "individual";
        case SessionType::PAIR: return "pair";
        case SessionType::SMALL_GROUP: return "small_group";
        case SessionType::LARGE_GROUP: return "large_group";
        default: return "pair";
    }
}

SessionType StudySessionPlan::stringToSessionType(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "individual") return SessionType::INDIVIDUAL;
    if (lower == "pair") return SessionType::PAIR;
    if (lower == "small_group") return SessionType::SMALL_GROUP;
    if (lower == "large_group") return SessionType::LARGE_GROUP;
    return SessionType::PAIR;
}

std::string StudySessionPlan::sessionStatusToString(SessionStatus status) {
    switch (status) {
        case SessionStatus::PLANNED: return "planned";
        case SessionStatus::IN_PROGRESS: return "in_progress";
        case SessionStatus::COMPLETED: return "completed";
        case SessionStatus::CANCELLED: return "cancelled";
        default: return "planned";
    }
}

SessionStatus StudySessionPlan::stringToSessionStatus(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "in_progress") return SessionStatus::IN_PROGRESS;
    if (lower == "completed") return SessionStatus::COMPLETED;
    if (lower == "cancelled") return SessionStatus::CANCELLED;
    return SessionStatus::PLANNED;
}

bool StudySessionPlan::canAddParticipant() const {
    return current_participants < max_participants &&
           status == SessionStatus::PLANNED;
}

bool StudySessionPlan::isUpcoming() const {
    std::time_t now = std::time(nullptr);
    return scheduled_start > now && status == SessionStatus::PLANNED;
}
