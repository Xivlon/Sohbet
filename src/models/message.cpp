#include "models/message.h"
#include <sstream>
#include <regex>
#include <algorithm>

namespace sohbet {

Message::Message()
    : id(0), conversation_id(0), sender_id(0), content(""), media_url(""),
      read_at(0), delivered_at(0), created_at(std::time(nullptr)),
      is_read_at_null(true), is_delivered_at_null(true) {
}

Message::Message(int id, int conversation_id, int sender_id, const std::string& content,
                 const std::string& media_url, std::time_t read_at, std::time_t delivered_at,
                 std::time_t created_at, bool is_read_at_null, bool is_delivered_at_null)
    : id(id), conversation_id(conversation_id), sender_id(sender_id), content(content),
      media_url(media_url), read_at(read_at), delivered_at(delivered_at),
      created_at(created_at), is_read_at_null(is_read_at_null), 
      is_delivered_at_null(is_delivered_at_null) {
}

std::string Message::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << id << ",";
    oss << "\"conversation_id\":" << conversation_id << ",";
    oss << "\"sender_id\":" << sender_id << ",";
    oss << "\"content\":\"" << content << "\",";
    
    if (!media_url.empty()) {
        oss << "\"media_url\":\"" << media_url << "\",";
    } else {
        oss << "\"media_url\":null,";
    }
    
    // Convert time_t to ISO 8601 string
    if (!is_read_at_null) {
        char time_buf[30];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&read_at));
        oss << "\"read_at\":\"" << time_buf << "\",";
    } else {
        oss << "\"read_at\":null,";
    }
    
    if (!is_delivered_at_null) {
        char time_buf[30];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&delivered_at));
        oss << "\"delivered_at\":\"" << time_buf << "\",";
    } else {
        oss << "\"delivered_at\":null,";
    }
    
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&created_at));
    oss << "\"created_at\":\"" << time_buf << "\"";
    
    oss << "}";
    return oss.str();
}

Message Message::from_json(const std::string& json) {
    Message message;
    
    std::regex id_regex("\"id\"\\s*:\\s*(\\d+)");
    std::regex conv_regex("\"conversation_id\"\\s*:\\s*(\\d+)");
    std::regex sender_regex("\"sender_id\"\\s*:\\s*(\\d+)");
    std::regex content_regex("\"content\"\\s*:\\s*\"([^\"]+)\"");
    std::regex media_regex("\"media_url\"\\s*:\\s*\"([^\"]+)\"");
    
    std::smatch match;
    
    if (std::regex_search(json, match, id_regex)) {
        message.id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, conv_regex)) {
        message.conversation_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, sender_regex)) {
        message.sender_id = std::stoi(match[1].str());
    }
    if (std::regex_search(json, match, content_regex)) {
        message.content = match[1].str();
    }
    if (std::regex_search(json, match, media_regex)) {
        message.media_url = match[1].str();
    }
    
    return message;
}

} // namespace sohbet
