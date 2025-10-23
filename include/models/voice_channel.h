#ifndef VOICE_CHANNEL_H
#define VOICE_CHANNEL_H

#include <string>
#include <ctime>

namespace sohbet {

/**
 * @brief Represents a voice channel in the system
 * 
 * Voice channels are virtual rooms where users can connect for voice/video communication.
 * They can be temporary (deleted when empty) or permanent.
 */
class VoiceChannel {
public:
    int id;
    std::string name;
    std::string description;
    int creator_id;
    int murmur_channel_id; // ID in the external Murmur server, -1 if not created yet
    int max_users;
    bool is_temporary;
    std::time_t created_at;

    VoiceChannel();
    VoiceChannel(int id, const std::string& name, const std::string& description,
                 int creator_id, int murmur_channel_id, int max_users, 
                 bool is_temporary, std::time_t created_at);

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
