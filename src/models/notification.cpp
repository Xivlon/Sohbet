#include "models/notification.h"
#include <sstream>
#include <regex>

namespace sohbet {

Notification::Notification()
    : id(0), user_id(0), type(""), title(""), message(""),
      related_user_id(std::nullopt), related_post_id(std::nullopt),
      related_comment_id(std::nullopt), related_group_id(std::nullopt),
      related_session_id(std::nullopt), action_url(""), is_read(false),
      created_at(std::time(nullptr)), read_at(0), is_read_at_null(true) {
}

Notification::Notification(int id, int user_id, const std::string& type,
                           const std::string& title, const std::string& message,
                           std::optional<int> related_user_id,
                           std::optional<int> related_post_id,
                           std::optional<int> related_comment_id,
                           std::optional<int> related_group_id,
                           std::optional<int> related_session_id,
                           const std::string& action_url, bool is_read,
                           std::time_t created_at, std::time_t read_at,
                           bool is_read_at_null)
    : id(id), user_id(user_id), type(type), title(title), message(message),
      related_user_id(related_user_id), related_post_id(related_post_id),
      related_comment_id(related_comment_id), related_group_id(related_group_id),
      related_session_id(related_session_id), action_url(action_url),
      is_read(is_read), created_at(created_at), read_at(read_at),
      is_read_at_null(is_read_at_null) {
}

std::string Notification::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"user_id\":" << user_id << ",";
    oss << "\"type\":\"" << type << "\",";
    oss << "\"title\":\"" << title << "\",";
    oss << "\"message\":\"" << message << "\",";

    if (related_user_id.has_value()) {
        oss << "\"related_user_id\":" << related_user_id.value() << ",";
    } else {
        oss << "\"related_user_id\":null,";
    }

    if (related_post_id.has_value()) {
        oss << "\"related_post_id\":" << related_post_id.value() << ",";
    } else {
        oss << "\"related_post_id\":null,";
    }

    if (related_comment_id.has_value()) {
        oss << "\"related_comment_id\":" << related_comment_id.value() << ",";
    } else {
        oss << "\"related_comment_id\":null,";
    }

    if (related_group_id.has_value()) {
        oss << "\"related_group_id\":" << related_group_id.value() << ",";
    } else {
        oss << "\"related_group_id\":null,";
    }

    if (related_session_id.has_value()) {
        oss << "\"related_session_id\":" << related_session_id.value() << ",";
    } else {
        oss << "\"related_session_id\":null,";
    }

    if (!action_url.empty()) {
        oss << "\"action_url\":\"" << action_url << "\",";
    } else {
        oss << "\"action_url\":null,";
    }

    oss << "\"is_read\":" << (is_read ? "true" : "false") << ",";

    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\",";

    if (!is_read_at_null) {
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&read_at));
        oss << "\"read_at\":\"" << time_buf << "\"";
    } else {
        oss << "\"read_at\":null";
    }

    oss << "}";
    return oss.str();
}

Notification Notification::from_json(const std::string& json) {
    Notification notification;

    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex user_id_regex("\"user_id\"\\s*:\\s*(\\d+)");
    std::regex type_regex("\"type\"\\s*:\\s*\"([^\"]+)\"");
    std::regex title_regex("\"title\"\\s*:\\s*\"([^\"]+)\"");
    std::regex message_regex("\"message\"\\s*:\\s*\"([^\"]+)\"");

    std::smatch match;

    if (std::regex_search(json, match, id_regex)) {
        notification.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, user_id_regex)) {
        notification.user_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, type_regex)) {
        notification.type = match[1].str();
    }
    if (std::regex_search(json, match, title_regex)) {
        notification.title = match[1].str();
    }
    if (std::regex_search(json, match, message_regex)) {
        notification.message = match[1].str();
    }

    return notification;
}

} // namespace sohbet
