#include "voice/voice_service.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>

namespace sohbet {

// VoiceConnectionToken implementation
std::string VoiceConnectionToken::to_json() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"connection_token\":\"" << token << "\",";
    oss << "\"channel_id\":" << channel_id << ",";

    // Convert time_t to ISO 8601 string
    char time_buf[30];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&expires_at));
    oss << "\"expires_at\":\"" << time_buf << "\"";

    oss << "}";
    return oss.str();
}

// VoiceService implementation
VoiceService::VoiceService(const VoiceConfig& config)
    : config_(config) {
}

bool VoiceService::is_enabled() const {
    return config_.enabled && config_.is_valid();
}

// VoiceServiceStub implementation
VoiceServiceStub::VoiceServiceStub(const VoiceConfig& config)
    : VoiceService(config), next_channel_id_(1) {
}

VoiceChannel VoiceServiceStub::create_channel(const std::string& name,
                                              const std::string& channel_type,
                                              int group_id,
                                              int organization_id) {
    VoiceChannel channel;
    channel.id = next_channel_id_++;
    channel.name = name;
    channel.channel_type = channel_type;
    channel.group_id = group_id;
    channel.organization_id = organization_id;
    channel.created_at = std::time(nullptr);

    channels_.push_back(channel);
    return channel;
}

bool VoiceServiceStub::delete_channel(int channel_id) {
    auto it = std::find_if(channels_.begin(), channels_.end(),
                          [channel_id](const VoiceChannel& c) { return c.id == channel_id; });
    
    if (it != channels_.end()) {
        channels_.erase(it);
        return true;
    }
    return false;
}

std::vector<VoiceChannel> VoiceServiceStub::list_channels() {
    return channels_;
}

VoiceChannel VoiceServiceStub::get_channel(int channel_id) {
    auto it = std::find_if(channels_.begin(), channels_.end(),
                          [channel_id](const VoiceChannel& c) { return c.id == channel_id; });
    
    if (it != channels_.end()) {
        return *it;
    }
    
    // Return empty channel if not found
    return VoiceChannel();
}

VoiceConnectionToken VoiceServiceStub::generate_connection_token(int user_id, int channel_id) {
    VoiceConnectionToken token;

    // Generate a simple token (in production, use proper JWT or secure token generation)
    std::stringstream ss;
    ss << "webrtc_token_u" << user_id << "_c" << channel_id << "_t" << std::time(nullptr);

    // Simple hash using std::hash (good enough for stub implementation)
    std::hash<std::string> hasher;
    size_t hash_value = hasher(ss.str());

    std::stringstream token_ss;
    token_ss << std::hex << hash_value;
    token.token = token_ss.str();

    token.channel_id = channel_id;
    token.expires_at = std::time(nullptr) + config_.token_expiry_seconds;

    return token;
}

bool VoiceServiceStub::validate_token(const std::string& token) {
    // Stub implementation - in production, check against database
    // For now, just check if token is not empty
    return !token.empty();
}

void VoiceServiceStub::log_access(int user_id, int channel_id, const std::string& action) {
    // Stub implementation - in production, write to database
    // For now, just do nothing (could add logging here)
    (void)user_id;
    (void)channel_id;
    (void)action;
}

} // namespace sohbet
