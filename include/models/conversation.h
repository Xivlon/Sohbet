#ifndef SOHBET_MODELS_CONVERSATION_H
#define SOHBET_MODELS_CONVERSATION_H

#include <string>
#include <ctime>

namespace sohbet {

class Conversation {
public:
    int id;
    int user1_id;
    int user2_id;
    std::time_t created_at;
    std::time_t last_message_at;

    Conversation();
    Conversation(int id, int user1_id, int user2_id, 
                 std::time_t created_at, std::time_t last_message_at);

    std::string to_json() const;
    static Conversation from_json(const std::string& json);
};

} // namespace sohbet

#endif // SOHBET_MODELS_CONVERSATION_H
