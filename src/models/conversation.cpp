#include "models/conversation.h"
#include <sstream>
#include <regex>
#include <algorithm>

namespace sohbet {

Conversation::Conversation()
    : id(0), user1_id(0), user2_id(0), 
      created_at(std::time(nullptr)), last_message_at(std::time(nullptr)) {
}

Conversation::Conversation(int id, int user1_id, int user2_id,
                           std::time_t created_at, std::time_t last_message_at)
    : id(id), user1_id(user1_id), user2_id(user2_id),
      created_at(created_at), last_message_at(last_message_at) {
}

std::string Conversation::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"user1_id\":" << user1_id << ",";
    oss << "\"user2_id\":" << user2_id << ",";
    
    // Convert time_t to ISO 8601 string
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\",";
    
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&last_message_at));
    oss << "\"last_message_at\":\"" << time_buf << "\"";
    
    oss << "}";
    return oss.str();
}

Conversation Conversation::from_json(const std::string& json) {
    Conversation conversation;
    
    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex user1_regex("\"user1_id\"\\s*:\\s*(\\d+)");
    std::regex user2_regex("\"user2_id\"\\s*:\\s*(\\d+)");
    
    std::smatch match;
    
    if (std::regex_search(json, match, id_regex)) {
        conversation.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, user1_regex)) {
        conversation.user1_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, user2_regex)) {
        conversation.user2_id = std::stoi(match[1].str());
    }
    
    return conversation;
}

} // namespace sohbet
