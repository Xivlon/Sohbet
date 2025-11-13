#ifndef VOICE_CHANNEL_REPOSITORY_H
#define VOICE_CHANNEL_REPOSITORY_H

#include "db/database.h"
#include "models/voice_channel.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

/**
 * @brief Repository for managing voice channels in the database
 */
class VoiceChannelRepository {
private:
    std::shared_ptr<db::Database> database_;

public:
    explicit VoiceChannelRepository(std::shared_ptr<db::Database> database);

    /**
     * @brief Create a new voice channel in the database
     * @param channel VoiceChannel object to create (id will be set after creation)
     * @return Optional containing the created channel with id, or nullopt on failure
     */
    std::optional<VoiceChannel> create(VoiceChannel& channel);

    /**
     * @brief Find a voice channel by ID
     * @param id Voice channel ID
     * @return Optional containing the channel, or nullopt if not found
     */
    std::optional<VoiceChannel> findById(int id);

    /**
     * @brief List all active voice channels
     * @param limit Maximum number of channels to return
     * @param offset Number of channels to skip
     * @return Vector of voice channels
     */
    std::vector<VoiceChannel> findAll(int limit = 50, int offset = 0);

    /**
     * @brief Find channels by type (private, group, public)
     * @param channel_type Channel type to filter by
     * @param limit Maximum number of channels to return
     * @param offset Number of channels to skip
     * @return Vector of voice channels
     */
    std::vector<VoiceChannel> findByType(const std::string& channel_type, int limit = 50, int offset = 0);

    /**
     * @brief Delete a voice channel
     * @param id Voice channel ID
     * @return true if deleted successfully
     */
    bool deleteById(int id);

    /**
     * @brief Record a voice session (user joining a channel)
     * @param channel_id Voice channel ID
     * @param user_id User ID
     * @return Session ID if created successfully, or 0 on failure
     */
    int createSession(int channel_id, int user_id);

    /**
     * @brief End a voice session (user leaving a channel)
     * @param session_id Session ID
     * @return true if updated successfully
     */
    bool endSession(int session_id);

    /**
     * @brief Get active sessions for a channel
     * @param channel_id Voice channel ID
     * @return Number of active users in the channel
     */
    int getActiveUserCount(int channel_id);

    /**
     * @brief Get user's active session in a channel
     * @param user_id User ID
     * @param channel_id Voice channel ID
     * @return Session ID if found, or 0 if not found
     */
    int getUserActiveSession(int user_id, int channel_id);

    /**
     * @brief End all active sessions for a user (used when user disconnects)
     * @param user_id User ID
     * @return Number of sessions ended
     */
    int endAllUserSessions(int user_id);

    /**
     * @brief Find voice channels that have been empty for more than the specified duration
     * @param inactivity_minutes Number of minutes of inactivity to consider
     * @return Vector of channel IDs that are empty and inactive
     */
    std::vector<int> findEmptyInactiveChannels(int inactivity_minutes = 30);
};

} // namespace repositories
} // namespace sohbet

#endif // VOICE_CHANNEL_REPOSITORY_H
