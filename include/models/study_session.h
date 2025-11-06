#ifndef SOHBET_MODELS_STUDY_SESSION_H
#define SOHBET_MODELS_STUDY_SESSION_H

#include <string>
#include <ctime>
#include <optional>

namespace sohbet {

class StudySession {
public:
    int id;
    int group_id;
    std::string title;
    std::string description;
    std::string location;
    std::optional<int> voice_channel_id;
    std::time_t start_time;
    std::time_t end_time;
    int created_by;
    std::optional<int> max_participants;
    bool is_recurring;
    std::string recurrence_pattern;
    std::time_t created_at;
    std::time_t updated_at;
    bool is_voice_channel_null;
    bool is_max_participants_null;

    StudySession();
    StudySession(int id, int group_id, const std::string& title,
                 const std::string& description, const std::string& location,
                 std::optional<int> voice_channel_id, std::time_t start_time,
                 std::time_t end_time, int created_by,
                 std::optional<int> max_participants = std::nullopt,
                 bool is_recurring = false, const std::string& recurrence_pattern = "",
                 std::time_t created_at = 0, std::time_t updated_at = 0);

    std::string to_json() const;
    static StudySession from_json(const std::string& json);
};

} // namespace sohbet

#endif // SOHBET_MODELS_STUDY_SESSION_H
