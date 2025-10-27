#include "models/comment.h"
#include <sstream>

namespace sohbet {

Comment::Comment(int post_id, int author_id, const std::string& content)
    : post_id_(post_id), author_id_(author_id), content_(content) {
}

std::string Comment::toJson() const {
    std::ostringstream oss;
    oss << "{";
    
    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }
    
    oss << "\"post_id\":" << post_id_ << ",";
    
    if (parent_id_.has_value()) {
        oss << "\"parent_id\":" << parent_id_.value() << ",";
    } else {
        oss << "\"parent_id\":null,";
    }
    
    oss << "\"author_id\":" << author_id_ << ","
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
    
    oss << "\"";
    
    if (created_at_.has_value()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }
    
    if (updated_at_.has_value()) {
        oss << ",\"updated_at\":\"" << updated_at_.value() << "\"";
    }
    
    oss << "}";
    return oss.str();
}

Comment Comment::fromJson(const std::string& json) {
    // Basic JSON parsing (in production, use a proper JSON library)
    Comment comment;
    // TODO: Implement proper JSON parsing
    return comment;
}

} // namespace sohbet
