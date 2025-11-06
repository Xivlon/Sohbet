#include "models/hashtag.h"
#include <sstream>

namespace sohbet {

Hashtag::Hashtag(const std::string& tag)
    : tag_(tag), usage_count_(0) {
}

std::string Hashtag::toJson() const {
    std::ostringstream oss;
    oss << "{";

    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }

    oss << "\"tag\":\"" << tag_ << "\","
        << "\"usage_count\":" << usage_count_;

    if (created_at_.has_value()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }

    if (last_used_at_.has_value()) {
        oss << ",\"last_used_at\":\"" << last_used_at_.value() << "\"";
    }

    oss << "}";
    return oss.str();
}

Hashtag Hashtag::fromJson(const std::string& json) {
    (void)json; // Unused - TODO: Implement proper JSON parsing
    Hashtag hashtag;
    return hashtag;
}

} // namespace sohbet
