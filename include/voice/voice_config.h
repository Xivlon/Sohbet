#ifndef VOICE_CONFIG_H
#define VOICE_CONFIG_H

#include <string>

namespace sohbet {

/**
 * @brief Configuration for voice service integration
 * 
 * This structure holds all configuration needed to integrate with
 * an external voice server (e.g., Murmur/Mumble server).
 */
struct VoiceConfig {
    bool enabled;                    // Whether voice service is enabled
    std::string murmur_host;         // Hostname or IP of Murmur server
    int murmur_port;                 // Port for Murmur server (default: 64738)
    std::string murmur_admin_password; // Admin password for Murmur server
    int token_expiry_seconds;        // How long connection tokens are valid
    int max_users_per_channel;       // Default max users per channel
    bool enable_recording;           // Whether recording is enabled
    
    // Default constructor with sensible defaults
    VoiceConfig();
    
    /**
     * @brief Load configuration from environment variables or config file
     * @return true if configuration is valid, false otherwise
     */
    bool load_from_env();
    
    /**
     * @brief Validate the configuration
     * @return true if all required fields are set correctly
     */
    bool is_valid() const;
};

} // namespace sohbet

#endif // VOICE_CONFIG_H
