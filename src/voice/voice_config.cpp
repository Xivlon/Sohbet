#include "voice/voice_config.h"
#include <cstdlib>

namespace sohbet {

VoiceConfig::VoiceConfig()
    : enabled(false),
      murmur_host("0.0.0.0"),
      murmur_port(64738),
      murmur_admin_password(""),
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

    const char* env_host = std::getenv("SOHBET_MURMUR_HOST");
    if (env_host) {
        murmur_host = env_host;
    } else {
        // Auto-detect Fly.io hostname if running on Fly.io
        const char* fly_app_name = std::getenv("FLY_APP_NAME");
        if (fly_app_name) {
            // On Fly.io, construct the public hostname
            murmur_host = std::string(fly_app_name) + ".fly.dev";
        } else {
            // Try to get the public hostname from other sources
            const char* public_host = std::getenv("PUBLIC_HOSTNAME");
            if (public_host) {
                murmur_host = public_host;
            }
            // Otherwise keep the default from constructor
        }
    }

    const char* env_port = std::getenv("SOHBET_MURMUR_PORT");
    if (env_port) {
        murmur_port = std::atoi(env_port);
    }

    const char* env_password = std::getenv("SOHBET_MURMUR_ADMIN_PASSWORD");
    if (env_password) {
        murmur_admin_password = env_password;
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
    
    // Check required fields when enabled
    if (murmur_host.empty()) {
        return false;
    }
    
    if (murmur_port <= 0 || murmur_port > 65535) {
        return false;
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
