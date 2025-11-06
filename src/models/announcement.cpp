#include "models/announcement.h"
#include <sstream>

namespace sohbet {

Announcement::Announcement(int group_id, int author_id, const std::string& title, const std::string& content)
    : group_id_(group_id), author_id_(author_id), title_(title), content_(content), is_pinned_(false) {
}

std::string Announcement::toJson() const {
    std::ostringstream oss;
    oss << "{";

    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }

    oss << "\"group_id\":" << group_id_ << ","
        << "\"author_id\":" << author_id_ << ","
        << "\"title\":\"";

    // Escape quotes in title
    for (char c : title_) {
        if (c == '"') oss << "\\\"";
        else if (c == '\\') oss << "\\\\";
        else if (c == '\n') oss << "\\n";
        else if (c == '\r') oss << "\\r";
        else if (c == '\t') oss << "\\t";
        else oss << c;
    }

    oss << "\",\"content\":\"";

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
        << "\"is_pinned\":" << (is_pinned_ ? "true" : "false");

    if (created_at_.has_value()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }

    if (updated_at_.has_value()) {
        oss << ",\"updated_at\":\"" << updated_at_.value() << "\"";
    }

    // Include author information if available
    if (author_username_.has_value() || author_name_.has_value()) {
        oss << ",\"author\":{";
        oss << "\"id\":" << author_id_;

        if (author_username_.has_value()) {
            oss << ",\"username\":\"" << author_username_.value() << "\"";
        }

        if (author_name_.has_value()) {
            oss << ",\"name\":\"" << author_name_.value() << "\"";
        }

        oss << "}";
    }

    oss << "}";
    return oss.str();
}

Announcement Announcement::fromJson(const std::string& json) {
    (void)json; // Unused - TODO: Implement proper JSON parsing
    Announcement announcement;
    return announcement;
}

} // namespace sohbet
