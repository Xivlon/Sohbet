#include "repositories/notification_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

NotificationRepository::NotificationRepository(std::shared_ptr<db::Database> database)
    : database_(database) {
}

std::optional<Notification> NotificationRepository::createNotification(
    int user_id, const std::string& type, const std::string& title, const std::string& message,
    std::optional<int> related_user_id, std::optional<int> related_post_id,
    std::optional<int> related_comment_id, std::optional<int> related_group_id,
    std::optional<int> related_session_id, const std::string& action_url) {

    std::string query = "INSERT INTO notifications (user_id, type, title, message, "
                       "related_user_id, related_post_id, related_comment_id, "
                       "related_group_id, related_session_id, action_url) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) RETURNING id";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare create notification query" << std::endl;
        return std::nullopt;
    }

    stmt.bindInt(1, user_id);
    stmt.bindText(2, type);
    stmt.bindText(3, title);
    stmt.bindText(4, message);

    related_user_id.has_value() ? stmt.bindInt(5, related_user_id.value()) : stmt.bindNull(5);
    related_post_id.has_value() ? stmt.bindInt(6, related_post_id.value()) : stmt.bindNull(6);
    related_comment_id.has_value() ? stmt.bindInt(7, related_comment_id.value()) : stmt.bindNull(7);
    related_group_id.has_value() ? stmt.bindInt(8, related_group_id.value()) : stmt.bindNull(8);
    related_session_id.has_value() ? stmt.bindInt(9, related_session_id.value()) : stmt.bindNull(9);

    if (action_url.empty()) {
        stmt.bindNull(10);
    } else {
        stmt.bindText(10, action_url);
    }

    if (stmt.step() == SQLITE_ROW) {
        int notification_id = stmt.getInt(0);
        // Call step() again to commit the transaction
        stmt.step();
        return getById(notification_id);
    }

    std::cerr << "Failed to create notification" << std::endl;
    return std::nullopt;
}

std::optional<Notification> NotificationRepository::getById(int id) {
    std::string query = "SELECT id, user_id, type, title, message, "
                       "related_user_id, related_post_id, related_comment_id, "
                       "related_group_id, related_session_id, action_url, is_read, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM read_at)::bigint as read_at "
                       "FROM notifications WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return std::nullopt;
    }

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Notification notif;
        notif.id = stmt.getInt(0);
        notif.user_id = stmt.getInt(1);
        notif.type = stmt.getText(2);
        notif.title = stmt.getText(3);
        notif.message = stmt.getText(4);

        notif.related_user_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));
        notif.related_post_id = stmt.isNull(6) ? std::nullopt : std::optional<int>(stmt.getInt(6));
        notif.related_comment_id = stmt.isNull(7) ? std::nullopt : std::optional<int>(stmt.getInt(7));
        notif.related_group_id = stmt.isNull(8) ? std::nullopt : std::optional<int>(stmt.getInt(8));
        notif.related_session_id = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        if (!stmt.isNull(10)) {
            notif.action_url = stmt.getText(10);
        }

        notif.is_read = stmt.getInt(11) != 0;
        notif.created_at = stmt.getInt64(12);

        notif.is_read_at_null = stmt.isNull(13);
        if (!notif.is_read_at_null) {
            notif.read_at = stmt.getInt64(13);
        }

        return notif;
    }

    return std::nullopt;
}

std::vector<Notification> NotificationRepository::getUserNotifications(int user_id, int limit, int offset) {
    std::vector<Notification> notifications;

    std::string query = "SELECT id, user_id, type, title, message, "
                       "related_user_id, related_post_id, related_comment_id, "
                       "related_group_id, related_session_id, action_url, is_read, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM read_at)::bigint as read_at "
                       "FROM notifications "
                       "WHERE user_id = ? "
                       "ORDER BY created_at DESC "
                       "LIMIT ? OFFSET ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare get user notifications query" << std::endl;
        return notifications;
    }

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Notification notif;
        notif.id = stmt.getInt(0);
        notif.user_id = stmt.getInt(1);
        notif.type = stmt.getText(2);
        notif.title = stmt.getText(3);
        notif.message = stmt.getText(4);

        notif.related_user_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));
        notif.related_post_id = stmt.isNull(6) ? std::nullopt : std::optional<int>(stmt.getInt(6));
        notif.related_comment_id = stmt.isNull(7) ? std::nullopt : std::optional<int>(stmt.getInt(7));
        notif.related_group_id = stmt.isNull(8) ? std::nullopt : std::optional<int>(stmt.getInt(8));
        notif.related_session_id = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        if (!stmt.isNull(10)) {
            notif.action_url = stmt.getText(10);
        }

        notif.is_read = stmt.getInt(11) != 0;
        notif.created_at = stmt.getInt64(12);

        notif.is_read_at_null = stmt.isNull(13);
        if (!notif.is_read_at_null) {
            notif.read_at = stmt.getInt64(13);
        }

        notifications.push_back(notif);
    }

    return notifications;
}

std::vector<Notification> NotificationRepository::getUnreadNotifications(int user_id, int limit) {
    std::vector<Notification> notifications;

    std::string query = "SELECT id, user_id, type, title, message, "
                       "related_user_id, related_post_id, related_comment_id, "
                       "related_group_id, related_session_id, action_url, is_read, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM read_at)::bigint as read_at "
                       "FROM notifications "
                       "WHERE user_id = ? AND is_read = 0 "
                       "ORDER BY created_at DESC "
                       "LIMIT ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return notifications;
    }

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, limit);

    while (stmt.step() == SQLITE_ROW) {
        Notification notif;
        notif.id = stmt.getInt(0);
        notif.user_id = stmt.getInt(1);
        notif.type = stmt.getText(2);
        notif.title = stmt.getText(3);
        notif.message = stmt.getText(4);

        notif.related_user_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));
        notif.related_post_id = stmt.isNull(6) ? std::nullopt : std::optional<int>(stmt.getInt(6));
        notif.related_comment_id = stmt.isNull(7) ? std::nullopt : std::optional<int>(stmt.getInt(7));
        notif.related_group_id = stmt.isNull(8) ? std::nullopt : std::optional<int>(stmt.getInt(8));
        notif.related_session_id = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        if (!stmt.isNull(10)) {
            notif.action_url = stmt.getText(10);
        }

        notif.is_read = stmt.getInt(11) != 0;
        notif.created_at = stmt.getInt64(12);

        notif.is_read_at_null = stmt.isNull(13);
        if (!notif.is_read_at_null) {
            notif.read_at = stmt.getInt64(13);
        }

        notifications.push_back(notif);
    }

    return notifications;
}

int NotificationRepository::getUnreadCount(int user_id) {
    std::string query = "SELECT COUNT(*) FROM notifications WHERE user_id = ? AND is_read = 0";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return 0;
    }

    stmt.bindInt(1, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

bool NotificationRepository::markAsRead(int notification_id) {
    std::string query = "UPDATE notifications SET is_read = 1, read_at = CURRENT_TIMESTAMP "
                       "WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, notification_id);

    return stmt.step() == SQLITE_DONE;
}

bool NotificationRepository::markAllAsRead(int user_id) {
    std::string query = "UPDATE notifications SET is_read = 1, read_at = CURRENT_TIMESTAMP "
                       "WHERE user_id = ? AND is_read = 0";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool NotificationRepository::deleteNotification(int notification_id) {
    std::string query = "DELETE FROM notifications WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, notification_id);

    return stmt.step() == SQLITE_DONE;
}

bool NotificationRepository::deleteAllForUser(int user_id) {
    std::string query = "DELETE FROM notifications WHERE user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, user_id);

    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
