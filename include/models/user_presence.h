#ifndef SOHBET_MODELS_USER_PRESENCE_H
#define SOHBET_MODELS_USER_PRESENCE_H

#include <string>
#include <ctime>

namespace sohbet {

class UserPresence {
public:
    int id;
    int user_id;
    std::string status;  // 'online', 'away', 'busy', 'offline'
    std::string custom_status;
    std::time_t last_seen;
    std::time_t updated_at;

    UserPresence();
    UserPresence(int id, int user_id, const std::string& status,
                 const std::string& custom_status = "",
                 std::time_t last_seen = 0, std::time_t updated_at = 0);

    std::string to_json() const;
    static UserPresence from_json(const std::string& json);
};

} // namespace sohbet

#endif // SOHBET_MODELS_USER_PRESENCE_H
