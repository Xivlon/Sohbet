#ifndef STUDY_SESSION_PLAN_H
#define STUDY_SESSION_PLAN_H

#include <string>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class SessionType {
    INDIVIDUAL,
    PAIR,
    SMALL_GROUP,
    LARGE_GROUP
};

enum class SessionStatus {
    PLANNED,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED
};

class StudySessionPlan {
public:
    int id;
    int creator_id;

    // Session details
    std::string title;
    std::string description;
    std::string course_topic;
    SessionType session_type;

    // Scheduling
    std::time_t scheduled_start;
    std::time_t scheduled_end;
    std::string location;
    std::string meeting_link;

    // Participants
    int max_participants;
    int current_participants;

    // Status
    SessionStatus status;
    int completion_rating; // 1-5
    std::string completion_notes;

    // Metadata
    std::time_t created_at;
    std::time_t updated_at;

    StudySessionPlan();

    // Conversion methods
    json toJson() const;
    static StudySessionPlan fromJson(const json& j);

    // Helper methods
    static std::string sessionTypeToString(SessionType type);
    static SessionType stringToSessionType(const std::string& str);

    static std::string sessionStatusToString(SessionStatus status);
    static SessionStatus stringToSessionStatus(const std::string& str);

    bool canAddParticipant() const;
    bool isUpcoming() const;
};

#endif // STUDY_SESSION_PLAN_H
