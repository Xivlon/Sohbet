#ifndef SOHBET_REPOSITORIES_USER_PRESENCE_REPOSITORY_H
#define SOHBET_REPOSITORIES_USER_PRESENCE_REPOSITORY_H

#include "db/database.h"
#include "models/user_presence.h"
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

class UserPresenceRepository {
public:
    explicit UserPresenceRepository(std::shared_ptr<db::Database> database);

    // Update or create user presence
    std::optional<UserPresence> updatePresence(int user_id, const std::string& status,
                                               const std::string& custom_status = "");

    // Get presence for a user
    std::optional<UserPresence> getByUserId(int user_id);

    // Get presence for multiple users
    std::vector<UserPresence> getByUserIds(const std::vector<int>& user_ids);

    // Get all online users
    std::vector<UserPresence> getOnlineUsers();

    // Update last seen timestamp
    bool updateLastSeen(int user_id);

    // Set user as offline
    bool setOffline(int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet

#endif // SOHBET_REPOSITORIES_USER_PRESENCE_REPOSITORY_H
