#include "repositories/friendship_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

FriendshipRepository::FriendshipRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Friendship> FriendshipRepository::create(Friendship& friendship) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO friendships (requester_id, addressee_id, status)
        VALUES (?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, friendship.getRequesterId());
    stmt.bindInt(2, friendship.getAddresseeId());
    stmt.bindText(3, friendship.getStatus());

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        friendship.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return friendship;
    }

    return std::nullopt;
}

std::optional<Friendship> FriendshipRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, requester_id, addressee_id, status, created_at, updated_at
        FROM friendships WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Friendship friendship;
        friendship.setId(stmt.getInt(0));
        friendship.setRequesterId(stmt.getInt(1));
        friendship.setAddresseeId(stmt.getInt(2));
        friendship.setStatus(stmt.getText(3));
        friendship.setCreatedAt(stmt.getText(4));
        friendship.setUpdatedAt(stmt.getText(5));
        return friendship;
    }

    return std::nullopt;
}

std::vector<Friendship> FriendshipRepository::findByUserId(int user_id, const std::string& status) {
    std::vector<Friendship> friendships;
    if (!database_ || !database_->isOpen()) return friendships;

    std::string sql = R"(
        SELECT id, requester_id, addressee_id, status, created_at, updated_at
        FROM friendships 
        WHERE (requester_id = ? OR addressee_id = ?)
    )";
    
    if (!status.empty()) {
        sql += " AND status = ?";
    }
    
    sql += " ORDER BY created_at DESC";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return friendships;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, user_id);
    if (!status.empty()) {
        stmt.bindText(3, status);
    }

    while (stmt.step() == SQLITE_ROW) {
        Friendship friendship;
        friendship.setId(stmt.getInt(0));
        friendship.setRequesterId(stmt.getInt(1));
        friendship.setAddresseeId(stmt.getInt(2));
        friendship.setStatus(stmt.getText(3));
        friendship.setCreatedAt(stmt.getText(4));
        friendship.setUpdatedAt(stmt.getText(5));
        friendships.push_back(friendship);
    }

    return friendships;
}

std::vector<Friendship> FriendshipRepository::findPendingRequestsForUser(int user_id) {
    std::vector<Friendship> friendships;
    if (!database_ || !database_->isOpen()) return friendships;

    const std::string sql = R"(
        SELECT id, requester_id, addressee_id, status, created_at, updated_at
        FROM friendships 
        WHERE addressee_id = ? AND status = 'pending'
        ORDER BY created_at DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return friendships;

    stmt.bindInt(1, user_id);

    while (stmt.step() == SQLITE_ROW) {
        Friendship friendship;
        friendship.setId(stmt.getInt(0));
        friendship.setRequesterId(stmt.getInt(1));
        friendship.setAddresseeId(stmt.getInt(2));
        friendship.setStatus(stmt.getText(3));
        friendship.setCreatedAt(stmt.getText(4));
        friendship.setUpdatedAt(stmt.getText(5));
        friendships.push_back(friendship);
    }

    return friendships;
}

std::vector<Friendship> FriendshipRepository::findSentRequestsByUser(int user_id) {
    std::vector<Friendship> friendships;
    if (!database_ || !database_->isOpen()) return friendships;

    const std::string sql = R"(
        SELECT id, requester_id, addressee_id, status, created_at, updated_at
        FROM friendships 
        WHERE requester_id = ? AND status = 'pending'
        ORDER BY created_at DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return friendships;

    stmt.bindInt(1, user_id);

    while (stmt.step() == SQLITE_ROW) {
        Friendship friendship;
        friendship.setId(stmt.getInt(0));
        friendship.setRequesterId(stmt.getInt(1));
        friendship.setAddresseeId(stmt.getInt(2));
        friendship.setStatus(stmt.getText(3));
        friendship.setCreatedAt(stmt.getText(4));
        friendship.setUpdatedAt(stmt.getText(5));
        friendships.push_back(friendship);
    }

    return friendships;
}

std::vector<Friendship> FriendshipRepository::findFriendshipsForUser(int user_id) {
    return findByUserId(user_id, "accepted");
}

std::optional<Friendship> FriendshipRepository::findBetweenUsers(int user1_id, int user2_id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, requester_id, addressee_id, status, created_at, updated_at
        FROM friendships 
        WHERE (requester_id = ? AND addressee_id = ?) 
           OR (requester_id = ? AND addressee_id = ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, user1_id);
    stmt.bindInt(2, user2_id);
    stmt.bindInt(3, user2_id);
    stmt.bindInt(4, user1_id);

    if (stmt.step() == SQLITE_ROW) {
        Friendship friendship;
        friendship.setId(stmt.getInt(0));
        friendship.setRequesterId(stmt.getInt(1));
        friendship.setAddresseeId(stmt.getInt(2));
        friendship.setStatus(stmt.getText(3));
        friendship.setCreatedAt(stmt.getText(4));
        friendship.setUpdatedAt(stmt.getText(5));
        return friendship;
    }

    return std::nullopt;
}

bool FriendshipRepository::update(const Friendship& friendship) {
    if (!database_ || !database_->isOpen()) return false;
    if (!friendship.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE friendships 
        SET status = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, friendship.getStatus());
    stmt.bindInt(2, friendship.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool FriendshipRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM friendships WHERE id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool FriendshipRepository::areFriends(int user1_id, int user2_id) {
    auto friendship = findBetweenUsers(user1_id, user2_id);
    return friendship.has_value() && friendship->getStatus() == Friendship::STATUS_ACCEPTED;
}

bool FriendshipRepository::acceptRequest(int friendship_id) {
    auto friendship = findById(friendship_id);
    if (!friendship.has_value()) return false;
    
    friendship->setStatus(Friendship::STATUS_ACCEPTED);
    return update(friendship.value());
}

bool FriendshipRepository::rejectRequest(int friendship_id) {
    auto friendship = findById(friendship_id);
    if (!friendship.has_value()) return false;
    
    friendship->setStatus(Friendship::STATUS_REJECTED);
    return update(friendship.value());
}

std::vector<User> FriendshipRepository::getFriendsForUser(int user_id) {
    std::vector<User> friends;
    if (!database_ || !database_->isOpen()) return friends;

    const std::string sql = R"(
        SELECT u.id, u.username, u.email, u.name, u.position, u.phone_number,
               u.university, u.department, u.enrollment_year, u.created_at,
               u.warnings, u.primary_language, u.role, u.avatar_url, u.banner_url
        FROM users u
        JOIN friendships f ON (f.requester_id = u.id OR f.addressee_id = u.id)
        WHERE (f.requester_id = ? OR f.addressee_id = ?)
          AND f.status = 'accepted'
          AND u.id != ?
        ORDER BY u.username
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return friends;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, user_id);
    stmt.bindInt(3, user_id);

    while (stmt.step() == SQLITE_ROW) {
        User user;
        user.setId(stmt.getInt(0));
        user.setUsername(stmt.getText(1));
        user.setEmail(stmt.getText(2));
        user.setName(stmt.getText(3));
        user.setPosition(stmt.getText(4));
        user.setPhoneNumber(stmt.getText(5));
        user.setUniversity(stmt.getText(6));
        user.setDepartment(stmt.getText(7));
        if (!stmt.isNull(8)) {
            user.setEnrollmentYear(stmt.getInt(8));
        }
        user.setCreatedAt(stmt.getText(9));
        if (!stmt.isNull(10)) {
            user.setWarnings(stmt.getInt(10));
        }
        user.setPrimaryLanguage(stmt.getText(11));
        user.setRole(stmt.getText(12));
        user.setAvatarUrl(stmt.getText(13));
        user.setBannerUrl(stmt.getText(14));
        friends.push_back(user);
    }

    return friends;
}

} // namespace repositories
} // namespace sohbet
