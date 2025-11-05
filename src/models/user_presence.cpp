#include "models/user_presence.h"
#include <sstream>
#include <regex>

namespace sohbet {

UserPresence::UserPresence()
    : id(0), user_id(0), status("offline"), custom_status(""),
      last_seen(std::time(nullptr)), updated_at(std::time(nullptr)) {
}

UserPresence::UserPresence(int id, int user_id, const std::string& status,
                           const std::string& custom_status,
                           std::time_t last_seen, std::time_t updated_at)
    : id(id), user_id(user_id), status(status), custom_status(custom_status),
      last_seen(last_seen), updated_at(updated_at) {
}

std::string UserPresence::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"user_id\":" << user_id << ",";
    oss << "\"status\":\"" << status << "\",";

    if (!custom_status.empty()) {
        oss << "\"custom_status\":\"" << custom_status << "\",";
    } else {
        oss << "\"custom_status\":null,";
    }

    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&last_seen));
    oss << "\"last_seen\":\"" << time_buf << "\",";

    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&updated_at));
    oss << "\"updated_at\":\"" << time_buf << "\"";

    oss << "}";
    return oss.str();
}

UserPresence UserPresence::from_json(const std::string& json) {
    UserPresence presence;

    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex user_id_regex("\"user_id\"\\s*:\\s*(\\d+)");
    std::regex status_regex("\"status\"\\s*:\\s*\"([^\"]+)\"");
    std::regex custom_status_regex("\"custom_status\"\\s*:\\s*\"([^\"]+)\"");

    std::smatch match;

    if (std::regex_search(json, match, id_regex)) {
        presence.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, user_id_regex)) {
        presence.user_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, status_regex)) {
        presence.status = match[1].str();
    }
    if (std::regex_search(json, match, custom_status_regex)) {
        presence.custom_status = match[1].str();
    }

    return presence;
}

} // namespace sohbet
