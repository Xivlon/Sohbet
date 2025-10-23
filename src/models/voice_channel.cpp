#include "models/voice_channel.h"
#include <ctime>
#include <sstream>
#include <regex>

namespace sohbet {

VoiceChannel::VoiceChannel()
    : id(0), name(""), description(""), creator_id(0), 
      murmur_channel_id(-1), max_users(25), is_temporary(false),
      created_at(std::time(nullptr)) {
}

VoiceChannel::VoiceChannel(int id, const std::string& name, const std::string& description,
                           int creator_id, int murmur_channel_id, int max_users,
                           bool is_temporary, std::time_t created_at)
    : id(id), name(name), description(description), creator_id(creator_id),
      murmur_channel_id(murmur_channel_id), max_users(max_users),
      is_temporary(is_temporary), created_at(created_at) {
}

std::string VoiceChannel::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"channel_id\":" << id << ",";
    oss << "\"name\":\"" << name << "\",";
    oss << "\"description\":\"" << description << "\",";
    oss << "\"creator_id\":" << creator_id << ",";
    oss << "\"murmur_channel_id\":" << murmur_channel_id << ",";
    oss << "\"max_users\":" << max_users << ",";
    oss << "\"is_temporary\":" << (is_temporary ? "true" : "false") << ",";
    
    // Convert time_t to ISO 8601 string
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\"";
    
    oss << "}";
    return oss.str();
}

VoiceChannel VoiceChannel::from_json(const std::string& json) {
    VoiceChannel channel;
    
    std::regex id_regex("\"channel_id\"\\s*:\\s*(\\d+)");
    std::regex name_regex("\"name\"\\s*:\\s*\"([^\"]+)\"");
    std::regex desc_regex("\"description\"\\s*:\\s*\"([^\"]+)\"");
    std::regex creator_regex("\"creator_id\"\\s*:\\s*(\\d+)");
    std::regex murmur_regex("\"murmur_channel_id\"\\s*:\\s*(-?\\d+)");
    std::regex max_users_regex("\"max_users\"\\s*:\\s*(\\d+)");
    std::regex temp_regex("\"is_temporary\"\\s*:\\s*(true|false)");
    
    std::smatch match;
    
    if (std::regex_search(json, match, id_regex)) {
        channel.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, name_regex)) {
        channel.name = match[1].str();
    }
    if (std::regex_search(json, match, desc_regex)) {
        channel.description = match[1].str();
    }
    if (std::regex_search(json, match, creator_regex)) {
        channel.creator_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, murmur_regex)) {
        channel.murmur_channel_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, max_users_regex)) {
        channel.max_users = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, temp_regex)) {
        channel.is_temporary = (match[1].str() == "true");
    }
    
    return channel;
}

} // namespace sohbet
