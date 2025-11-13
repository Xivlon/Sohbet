#ifndef VOICE_CHANNEL_H
#define VOICE_CHANNEL_H

#include <string>
#include <ctime>

namespace sohbet {

/**
 * @brief Represents a voice channel in the system
 *
 * Voice channels are virtual rooms where users can connect for voice/video communication
 * using WebRTC peer-to-peer connections.
 * They can be private, group-based, or public (Khave).
 */
class VoiceChannel {
public:
    int id;
    std::string name;
    std::string channel_type;  // 'private', 'group', 'public' (Khave)
    int group_id;
    int organization_id;
    std::time_t created_at;

    VoiceChannel();

    /**
     * @brief Convert the voice channel to JSON format
     * @return JSON string representation of the voice channel
     */
    std::string to_json() const;

    /**
     * @brief Create a VoiceChannel from JSON data
     * @param json JSON string containing voice channel data
     * @return VoiceChannel object
     */
    static VoiceChannel from_json(const std::string& json);
};

} // namespace sohbet

#endif // VOICE_CHANNEL_H
