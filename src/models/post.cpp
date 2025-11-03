#include "models/post.h"
#include <sstream>

namespace sohbet {

Post::Post(int author_id, const std::string& content)
    : author_id_(author_id), content_(content) {
}

std::string Post::toJson() const {
    std::ostringstream oss;
    oss << "{";

    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }

    oss << "\"author_id\":" << author_id_ << ","
        << "\"author_type\":\"" << author_type_ << "\","
        << "\"content\":\"";

    // Escape quotes in content
    for (char c : content_) {
        if (c == '"') oss << "\\\"";
        else if (c == '\\') oss << "\\\\";
        else if (c == '\n') oss << "\\n";
        else if (c == '\r') oss << "\\r";
        else if (c == '\t') oss << "\\t";
        else oss << c;
    }

    oss << "\","
        << "\"visibility\":\"" << visibility_ << "\"";

    if (media_urls_.has_value()) {
        oss << ",\"media_urls\":" << media_urls_.value();
    }

    if (group_id_.has_value()) {
        oss << ",\"group_id\":" << group_id_.value();
    }

    if (created_at_.has_value()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }

    if (updated_at_.has_value()) {
        oss << ",\"updated_at\":\"" << updated_at_.value() << "\"";
    }

    // Include author information if available
    if (author_username_.has_value() || author_name_.has_value() || author_avatar_url_.has_value()) {
        oss << ",\"author\":{";
        oss << "\"id\":" << author_id_;

        if (author_username_.has_value()) {
            oss << ",\"username\":\"" << author_username_.value() << "\"";
        }

        if (author_name_.has_value()) {
            oss << ",\"name\":\"" << author_name_.value() << "\"";
        }

        if (author_avatar_url_.has_value()) {
            oss << ",\"avatar_url\":\"" << author_avatar_url_.value() << "\"";
        }

        oss << "}";
    }

    oss << "}";
    return oss.str();
}

Post Post::fromJson(const std::string& json) {
    (void)json; // Unused - TODO: Implement proper JSON parsing
    // Basic JSON parsing (in production, use a proper JSON library)
    Post post;
    return post;
}

} // namespace sohbet
