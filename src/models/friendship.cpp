#include "models/friendship.h"
#include <sstream>
#include <stdexcept>

namespace sohbet {

Friendship::Friendship(int requester_id, int addressee_id)
    : requester_id_(requester_id), addressee_id_(addressee_id), status_(STATUS_PENDING) {
}

std::string Friendship::toJson() const {
    std::ostringstream oss;
    oss << "{";
    
    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }
    
    oss << "\"requester_id\":" << requester_id_ << ","
        << "\"addressee_id\":" << addressee_id_ << ","
        << "\"status\":\"" << status_ << "\"";
    
    if (created_at_.has_value()) {
        oss << ",\"created_at\":\"" << created_at_.value() << "\"";
    }
    
    if (updated_at_.has_value()) {
        oss << ",\"updated_at\":\"" << updated_at_.value() << "\"";
    }
    
    oss << "}";
    return oss.str();
}

Friendship Friendship::fromJson(const std::string& json) {
    // Basic JSON parsing (in production, use a proper JSON library)
    Friendship friendship;
    // TODO: Implement proper JSON parsing
    return friendship;
}

} // namespace sohbet
