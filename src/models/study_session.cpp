#include "models/study_session.h"
#include <sstream>
#include <regex>

namespace sohbet {

StudySession::StudySession()
    : id(0), group_id(0), title(""), description(""), location(""),
      voice_channel_id(std::nullopt), start_time(0), end_time(0),
      created_by(0), max_participants(std::nullopt), is_recurring(false),
      recurrence_pattern(""), created_at(std::time(nullptr)),
      updated_at(std::time(nullptr)), is_voice_channel_null(true),
      is_max_participants_null(true) {
}

StudySession::StudySession(int id, int group_id, const std::string& title,
                           const std::string& description, const std::string& location,
                           std::optional<int> voice_channel_id, std::time_t start_time,
                           std::time_t end_time, int created_by,
                           std::optional<int> max_participants, bool is_recurring,
                           const std::string& recurrence_pattern,
                           std::time_t created_at, std::time_t updated_at)
    : id(id), group_id(group_id), title(title), description(description),
      location(location), voice_channel_id(voice_channel_id),
      start_time(start_time), end_time(end_time), created_by(created_by),
      max_participants(max_participants), is_recurring(is_recurring),
      recurrence_pattern(recurrence_pattern), created_at(created_at),
      updated_at(updated_at), is_voice_channel_null(!voice_channel_id.has_value()),
      is_max_participants_null(!max_participants.has_value()) {
}

std::string StudySession::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"group_id\":" << group_id << ",";
    oss << "\"title\":\"" << title << "\",";
    oss << "\"description\":\"" << description << "\",";
    oss << "\"location\":\"" << location << "\",";

    if (voice_channel_id.has_value()) {
        oss << "\"voice_channel_id\":" << voice_channel_id.value() << ",";
    } else {
        oss << "\"voice_channel_id\":null,";
    }

    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&start_time));
    oss << "\"start_time\":\"" << time_buf << "\",";

    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&end_time));
    oss << "\"end_time\":\"" << time_buf << "\",";

    oss << "\"created_by\":" << created_by << ",";

    if (max_participants.has_value()) {
        oss << "\"max_participants\":" << max_participants.value() << ",";
    } else {
        oss << "\"max_participants\":null,";
    }

    oss << "\"is_recurring\":" << (is_recurring ? "true" : "false") << ",";

    if (!recurrence_pattern.empty()) {
        oss << "\"recurrence_pattern\":\"" << recurrence_pattern << "\",";
    } else {
        oss << "\"recurrence_pattern\":null,";
    }

    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\",";

    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&updated_at));
    oss << "\"updated_at\":\"" << time_buf << "\"";

    oss << "}";
    return oss.str();
}

StudySession StudySession::from_json(const std::string& json) {
    StudySession session;

    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex group_id_regex("\"group_id\"\\s*:\\s*(\\d+)");
    std::regex title_regex("\"title\"\\s*:\\s*\"([^\"]+)\"");
    std::regex description_regex("\"description\"\\s*:\\s*\"([^\"]+)\"");
    std::regex location_regex("\"location\"\\s*:\\s*\"([^\"]+)\"");

    std::smatch match;

    if (std::regex_search(json, match, id_regex)) {
        session.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, group_id_regex)) {
        session.group_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, title_regex)) {
        session.title = match[1].str();
    }
    if (std::regex_search(json, match, description_regex)) {
        session.description = match[1].str();
    }
    if (std::regex_search(json, match, location_regex)) {
        session.location = match[1].str();
    }

    return session;
}

} // namespace sohbet
