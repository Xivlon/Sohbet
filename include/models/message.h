#ifndef SOHBET_MODELS_MESSAGE_H
#define SOHBET_MODELS_MESSAGE_H

#include <string>
#include <ctime>

namespace sohbet {

class Message {
public:
    int id;
    int conversation_id;
    int sender_id;
    std::string content;
    std::string media_url;
    std::time_t read_at;
    std::time_t delivered_at;
    std::time_t created_at;
    bool is_read_at_null;
    bool is_delivered_at_null;

    Message();
    Message(int id, int conversation_id, int sender_id, const std::string& content,
            const std::string& media_url, std::time_t read_at, std::time_t delivered_at,
            std::time_t created_at, bool is_read_at_null = true, bool is_delivered_at_null = true);

    std::string to_json() const;
    static Message from_json(const std::string& json);
};

} // namespace sohbet

#endif // SOHBET_MODELS_MESSAGE_H
