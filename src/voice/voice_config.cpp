#include "voice/voice_config.h"
#include <cstdlib>

namespace sohbet {

VoiceConfig::VoiceConfig()
    : enabled(false),
      token_expiry_seconds(300),
      max_users_per_channel(25),
      enable_recording(false) {
}

bool VoiceConfig::load_from_env() {
    // Load from environment variables
    const char* env_enabled = std::getenv("SOHBET_VOICE_ENABLED");
    if (env_enabled) {
        enabled = (std::string(env_enabled) == "true" || std::string(env_enabled) == "1");
    }

    const char* env_token_expiry = std::getenv("SOHBET_VOICE_TOKEN_EXPIRY");
    if (env_token_expiry) {
        token_expiry_seconds = std::atoi(env_token_expiry);
    }

    const char* env_max_users = std::getenv("SOHBET_VOICE_MAX_USERS");
    if (env_max_users) {
        max_users_per_channel = std::atoi(env_max_users);
    }

    const char* env_recording = std::getenv("SOHBET_VOICE_ENABLE_RECORDING");
    if (env_recording) {
        enable_recording = (std::string(env_recording) == "true" || std::string(env_recording) == "1");
    }

    return is_valid();
}

bool VoiceConfig::is_valid() const {
    // If not enabled, we don't need to validate other fields
    if (!enabled) {
        return true;
    }

    if (token_expiry_seconds <= 0) {
        return false;
    }

    if (max_users_per_channel <= 0) {
        return false;
    }

    return true;
}

} // namespace sohbet
