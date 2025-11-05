#include "repositories/user_presence_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

UserPresenceRepository::UserPresenceRepository(std::shared_ptr<db::Database> database)
    : database_(database) {
}

std::optional<UserPresence> UserPresenceRepository::updatePresence(
    int user_id, const std::string& status, const std::string& custom_status) {

    std::string query = "INSERT INTO user_presence (user_id, status, custom_status, last_seen, updated_at) "
                       "VALUES (?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP) "
                       "ON CONFLICT(user_id) DO UPDATE SET "
                       "status = excluded.status, "
                       "custom_status = excluded.custom_status, "
                       "last_seen = CURRENT_TIMESTAMP, "
                       "updated_at = CURRENT_TIMESTAMP";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare update presence query" << std::endl;
        return std::nullopt;
    }

    stmt.bindInt(1, user_id);
    stmt.bindText(2, status);

    if (custom_status.empty()) {
        stmt.bindNull(3);
    } else {
        stmt.bindText(3, custom_status);
    }

    if (stmt.step() == SQLITE_DONE) {
        return getByUserId(user_id);
    }

    return std::nullopt;
}

std::optional<UserPresence> UserPresenceRepository::getByUserId(int user_id) {
    std::string query = "SELECT id, user_id, status, custom_status, "
                       "strftime('%s', last_seen) as last_seen, "
                       "strftime('%s', updated_at) as updated_at "
                       "FROM user_presence WHERE user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return std::nullopt;
    }

    stmt.bindInt(1, user_id);

    if (stmt.step() == SQLITE_ROW) {
        UserPresence presence;
        presence.id = stmt.getInt(0);
        presence.user_id = stmt.getInt(1);
        presence.status = stmt.getText(2);

        if (!stmt.isNull(3)) {
            presence.custom_status = stmt.getText(3);
        }

        presence.last_seen = stmt.getInt64(4);
        presence.updated_at = stmt.getInt64(5);

        return presence;
    }

    return std::nullopt;
}

std::vector<UserPresence> UserPresenceRepository::getByUserIds(const std::vector<int>& user_ids) {
    std::vector<UserPresence> presences;

    if (user_ids.empty()) {
        return presences;
    }

    std::string placeholders;
    for (size_t i = 0; i < user_ids.size(); ++i) {
        if (i > 0) placeholders += ",";
        placeholders += "?";
    }

    std::string query = "SELECT id, user_id, status, custom_status, "
                       "strftime('%s', last_seen) as last_seen, "
                       "strftime('%s', updated_at) as updated_at "
                       "FROM user_presence WHERE user_id IN (" + placeholders + ")";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return presences;
    }

    for (size_t i = 0; i < user_ids.size(); ++i) {
        stmt.bindInt(i + 1, user_ids[i]);
    }

    while (stmt.step() == SQLITE_ROW) {
        UserPresence presence;
        presence.id = stmt.getInt(0);
        presence.user_id = stmt.getInt(1);
        presence.status = stmt.getText(2);

        if (!stmt.isNull(3)) {
            presence.custom_status = stmt.getText(3);
        }

        presence.last_seen = stmt.getInt64(4);
        presence.updated_at = stmt.getInt64(5);

        presences.push_back(presence);
    }

    return presences;
}

std::vector<UserPresence> UserPresenceRepository::getOnlineUsers() {
    std::vector<UserPresence> presences;

    std::string query = "SELECT id, user_id, status, custom_status, "
                       "strftime('%s', last_seen) as last_seen, "
                       "strftime('%s', updated_at) as updated_at "
                       "FROM user_presence "
                       "WHERE status IN ('online', 'away', 'busy') "
                       "ORDER BY updated_at DESC";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return presences;
    }

    while (stmt.step() == SQLITE_ROW) {
        UserPresence presence;
        presence.id = stmt.getInt(0);
        presence.user_id = stmt.getInt(1);
        presence.status = stmt.getText(2);

        if (!stmt.isNull(3)) {
            presence.custom_status = stmt.getText(3);
        }

        presence.last_seen = stmt.getInt64(4);
        presence.updated_at = stmt.getInt64(5);

        presences.push_back(presence);
    }

    return presences;
}

bool UserPresenceRepository::updateLastSeen(int user_id) {
    std::string query = "UPDATE user_presence SET last_seen = CURRENT_TIMESTAMP, "
                       "updated_at = CURRENT_TIMESTAMP WHERE user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool UserPresenceRepository::setOffline(int user_id) {
    std::string query = "UPDATE user_presence SET status = 'offline', "
                       "updated_at = CURRENT_TIMESTAMP WHERE user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, user_id);

    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
