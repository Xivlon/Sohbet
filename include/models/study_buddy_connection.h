#ifndef STUDY_BUDDY_CONNECTION_H
#define STUDY_BUDDY_CONNECTION_H

#include <string>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class StudyBuddyConnection {
public:
    int id;
    int user_id;
    int buddy_id;

    // Connection details
    std::time_t connected_at;
    std::time_t last_study_session;
    int total_study_sessions;
    int connection_strength; // 0-100

    // Connection preferences
    bool is_favorite;
    bool notification_enabled;

    // Metadata
    std::string notes;
    std::time_t created_at;
    std::time_t updated_at;

    StudyBuddyConnection();

    // Conversion methods
    json toJson() const;
    static StudyBuddyConnection fromJson(const json& j);

    // Helper methods
    void incrementStudySessions();
    void updateConnectionStrength(); // Calculates based on frequency and recency
};

#endif // STUDY_BUDDY_CONNECTION_H
