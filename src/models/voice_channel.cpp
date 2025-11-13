#include "models/voice_channel.h"
#include <ctime>
#include <sstream>
#include <regex>
#include <iomanip>

namespace sohbet {

VoiceChannel::VoiceChannel()
    : id(0), name(""), channel_type("public"),
      group_id(0), organization_id(0),
      created_at(std::time(nullptr)) {
}

std::string VoiceChannel::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"name\":\"" << name << "\",";
    oss << "\"channel_type\":\"" << channel_type << "\",";
    
    if (group_id > 0) {
        oss << "\"group_id\":" << group_id << ",";
    } else {
        oss << "\"group_id\":null,";
    }
    
    if (organization_id > 0) {
        oss << "\"organization_id\":" << organization_id << ",";
    } else {
        oss << "\"organization_id\":null,";
    }

    // Convert time_t to ISO 8601 string
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\"";
    
    oss << "}";
    return oss.str();
}

VoiceChannel VoiceChannel::from_json(const std::string& json) {
    VoiceChannel channel;

    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex name_regex("\"name\"\\s*:\\s*\"([^\"]+)\"");
    std::regex type_regex("\"channel_type\"\\s*:\\s*\"([^\"]+)\"");
    std::regex group_regex("\"group_id\"\\s*:\\s*(\\d+)");
    std::regex org_regex("\"organization_id\"\\s*:\\s*(\\d+)");

    std::smatch match;

    if (std::regex_search(json, match, id_regex)) {
        channel.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, name_regex)) {
        channel.name = match[1].str();
    }
    if (std::regex_search(json, match, type_regex)) {
        channel.channel_type = match[1].str();
    }
    if (std::regex_search(json, match, group_regex)) {
        channel.group_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, org_regex)) {
        channel.organization_id = std::stoi(match[1].str());
    }

    return channel;
}

} // namespace sohbet
