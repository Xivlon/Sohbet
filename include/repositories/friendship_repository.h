#pragma once

#include "models/friendship.h"
#include "models/user.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

class FriendshipRepository {
public:
    explicit FriendshipRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Friendship> create(Friendship& friendship);
    std::optional<Friendship> findById(int id);
    std::vector<Friendship> findByUserId(int user_id, const std::string& status = "");
    std::vector<Friendship> findPendingRequestsForUser(int user_id);
    std::vector<Friendship> findSentRequestsByUser(int user_id);
    std::vector<Friendship> findFriendshipsForUser(int user_id);
    std::optional<Friendship> findBetweenUsers(int user1_id, int user2_id);
    bool update(const Friendship& friendship);
    bool deleteById(int id);
    
    // Helper methods
    bool areFriends(int user1_id, int user2_id);
    bool acceptRequest(int friendship_id);
    bool rejectRequest(int friendship_id);
    
    // Get friends (users) for a specific user
    std::vector<User> getFriendsForUser(int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
