#ifndef VOICE_SERVICE_H
#define VOICE_SERVICE_H

#include "models/voice_channel.h"
#include "voice/voice_config.h"
#include <string>
#include <vector>
#include <memory>

namespace sohbet {

/**
 * @brief Connection token for joining a voice channel
 * WebRTC uses peer-to-peer connections, so no server host/port needed
 */
struct VoiceConnectionToken {
    std::string token;
    std::time_t expires_at;
    int channel_id;

    std::string to_json() const;
};

/**
 * @brief Interface for voice service operations
 *
 * This class provides an abstraction layer for managing voice channels
 * using WebRTC for peer-to-peer voice communication.
 *
 * The interface is designed to be extensible for future implementations.
 */
class VoiceService {
protected:
    VoiceConfig config_;

public:
    explicit VoiceService(const VoiceConfig& config);
    virtual ~VoiceService() = default;

    /**
     * @brief Check if voice service is enabled and configured
     * @return true if service is available
     */
    virtual bool is_enabled() const;

    /**
     * @brief Create a new voice channel
     * @param name Channel name
     * @param channel_type Type of channel ('private', 'group', 'public')
     * @param group_id Optional group ID (for group channels)
     * @param organization_id Optional organization ID
     * @return VoiceChannel object with ID assigned, or empty channel on failure
     */
    virtual VoiceChannel create_channel(const std::string& name,
                                       const std::string& channel_type,
                                       int group_id = 0,
                                       int organization_id = 0) = 0;

    /**
     * @brief Delete a voice channel
     * @param channel_id ID of the channel to delete
     * @return true if deletion was successful
     */
    virtual bool delete_channel(int channel_id) = 0;

    /**
     * @brief List all active voice channels
     * @return Vector of VoiceChannel objects
     */
    virtual std::vector<VoiceChannel> list_channels() = 0;

    /**
     * @brief Get a specific voice channel by ID
     * @param channel_id ID of the channel
     * @return VoiceChannel object, or empty channel if not found
     */
    virtual VoiceChannel get_channel(int channel_id) = 0;

    /**
     * @brief Generate a connection token for a user to join a channel
     * @param user_id ID of the user requesting access
     * @param channel_id ID of the channel to join
     * @return VoiceConnectionToken with connection details
     */
    virtual VoiceConnectionToken generate_connection_token(int user_id, int channel_id) = 0;

    /**
     * @brief Validate a connection token
     * @param token Token string to validate
     * @return true if token is valid and not expired
     */
    virtual bool validate_token(const std::string& token) = 0;

    /**
     * @brief Log a voice channel access event
     * @param user_id ID of the user
     * @param channel_id ID of the channel
     * @param action Action performed (join, leave, kicked, banned)
     */
    virtual void log_access(int user_id, int channel_id, const std::string& action) = 0;
};

/**
 * @brief In-memory implementation of VoiceService for WebRTC
 *
 * This implementation stores channels in memory and generates connection tokens.
 * WebRTC connections are peer-to-peer, so no external server is required.
 */
class VoiceServiceStub : public VoiceService {
private:
    std::vector<VoiceChannel> channels_;
    int next_channel_id_;

public:
    explicit VoiceServiceStub(const VoiceConfig& config);

    VoiceChannel create_channel(const std::string& name,
                               const std::string& channel_type,
                               int group_id = 0,
                               int organization_id = 0) override;

    bool delete_channel(int channel_id) override;
    std::vector<VoiceChannel> list_channels() override;
    VoiceChannel get_channel(int channel_id) override;
    VoiceConnectionToken generate_connection_token(int user_id, int channel_id) override;
    bool validate_token(const std::string& token) override;
    void log_access(int user_id, int channel_id, const std::string& action) override;
};

} // namespace sohbet

#endif // VOICE_SERVICE_H
