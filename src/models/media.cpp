#include "models/media.h"
#include <sstream>

namespace sohbet {

Media::Media(int user_id, const std::string& media_type, const std::string& storage_key)
    : user_id_(user_id), media_type_(media_type), storage_key_(storage_key) {}

std::string Media::toJson() const {
    std::ostringstream oss;
    oss << "{";
    
    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }
    
    oss << "\"user_id\":" << user_id_ << ",";
    oss << "\"media_type\":\"" << media_type_ << "\",";
    oss << "\"storage_key\":\"" << storage_key_ << "\"";
    
    if (file_name_.has_value() && !file_name_.value().empty()) {
        oss << ",\"file_name\":\"" << file_name_.value() << "\"";
    }
    
    if (file_size_.has_value()) {
        oss << ",\"file_size\":" << file_size_.value();
    }
    
    if (mime_type_.has_value() && !mime_type_.value().empty()) {
        oss << ",\"mime_type\":\"" << mime_type_.value() << "\"";
    }
    
    if (url_.has_value() && !url_.value().empty()) {
        oss << ",\"url\":\"" << url_.value() << "\"";
    }
    
    if (created_at_.has_value() && !created_at_.value().empty()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }
    
    oss << "}";
    return oss.str();
}

} // namespace sohbet
