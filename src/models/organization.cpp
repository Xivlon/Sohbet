#include "models/organization.h"
#include <sstream>

namespace sohbet {

Organization::Organization(const std::string& name, const std::string& type)
    : name_(name), type_(type) {}

std::string Organization::toJson() const {
    std::ostringstream oss;
    oss << "{";
    if (id_) {
        oss << "\"id\":" << *id_ << ",";
    }
    oss << "\"name\":\"" << name_ << "\",";
    oss << "\"type\":\"" << type_ << "\",";
    oss << "\"category\":\"" << type_ << "\"";  // Frontend expects 'category'
    if (description_) {
        oss << ",\"description\":\"" << *description_ << "\"";
    }
    if (email_) {
        oss << ",\"email\":\"" << *email_ << "\"";
    }
    if (website_) {
        oss << ",\"website\":\"" << *website_ << "\"";
    }
    if (logo_url_) {
        oss << ",\"logo_url\":\"" << *logo_url_ << "\"";
    }
    if (created_at_) {
        oss << ",\"created_at\":\"" << *created_at_ << "\"";
    }
    if (updated_at_) {
        oss << ",\"updated_at\":\"" << *updated_at_ << "\"";
    }
    oss << "}";
    return oss.str();
}

Organization Organization::fromJson(const std::string& json) {
    // Basic implementation - would need a proper JSON parser in production
    (void)json; // Suppress unused parameter warning
    Organization org;
    return org;
}

} // namespace sohbet
