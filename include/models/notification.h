#ifndef SOHBET_MODELS_NOTIFICATION_H
#define SOHBET_MODELS_NOTIFICATION_H

#include <string>
#include <ctime>
#include <optional>

namespace sohbet {

class Notification {
public:
    int id;
    int user_id;
    std::string type;
    std::string title;
    std::string message;
    std::optional<int> related_user_id;
    std::optional<int> related_post_id;
    std::optional<int> related_comment_id;
    std::optional<int> related_group_id;
    std::optional<int> related_session_id;
    std::string action_url;
    bool is_read;
    std::time_t created_at;
    std::time_t read_at;
    bool is_read_at_null;

    Notification();
    Notification(int id, int user_id, const std::string& type, const std::string& title,
                 const std::string& message, std::optional<int> related_user_id = std::nullopt,
                 std::optional<int> related_post_id = std::nullopt,
                 std::optional<int> related_comment_id = std::nullopt,
                 std::optional<int> related_group_id = std::nullopt,
                 std::optional<int> related_session_id = std::nullopt,
                 const std::string& action_url = "", bool is_read = false,
                 std::time_t created_at = 0, std::time_t read_at = 0, bool is_read_at_null = true);

    std::string to_json() const;
    static Notification from_json(const std::string& json);
};

} // namespace sohbet

#endif // SOHBET_MODELS_NOTIFICATION_H
