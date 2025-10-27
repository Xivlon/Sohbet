#include "models/role.h"
#include <sstream>

namespace sohbet {

Role::Role(const std::string& name, const std::string& description)
    : name_(name), description_(description) {}

std::string Role::toJson() const {
    std::ostringstream json;
    json << "{";
    
    if (id_.has_value()) {
        json << "\"id\":" << id_.value() << ",";
    }
    
    json << "\"name\":\"" << name_ << "\"";
    
    if (!description_.empty()) {
        json << ",\"description\":\"" << description_ << "\"";
    }
    
    if (created_at_.has_value()) {
        json << ",\"created_at\":\"" << created_at_.value() << "\"";
    }
    
    json << "}";
    return json.str();
}

// Simplified fromJson - in production, use a proper JSON library
Role Role::fromJson(const std::string& json) {
    Role role;
    // TODO: Implement proper JSON parsing
    return role;
}

} // namespace sohbet
