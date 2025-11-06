#ifndef SOHBET_REPOSITORIES_NOTIFICATION_REPOSITORY_H
#define SOHBET_REPOSITORIES_NOTIFICATION_REPOSITORY_H

#include "db/database.h"
#include "models/notification.h"
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

class NotificationRepository {
public:
    explicit NotificationRepository(std::shared_ptr<db::Database> database);

    // Create a new notification
    std::optional<Notification> createNotification(int user_id, const std::string& type,
                                                    const std::string& title, const std::string& message,
                                                    std::optional<int> related_user_id = std::nullopt,
                                                    std::optional<int> related_post_id = std::nullopt,
                                                    std::optional<int> related_comment_id = std::nullopt,
                                                    std::optional<int> related_group_id = std::nullopt,
                                                    std::optional<int> related_session_id = std::nullopt,
                                                    const std::string& action_url = "");

    // Get a notification by ID
    std::optional<Notification> getById(int id);

    // Get notifications for a user with pagination
    std::vector<Notification> getUserNotifications(int user_id, int limit = 50, int offset = 0);

    // Get unread notifications for a user
    std::vector<Notification> getUnreadNotifications(int user_id, int limit = 50);

    // Get unread notification count
    int getUnreadCount(int user_id);

    // Mark notification as read
    bool markAsRead(int notification_id);

    // Mark all notifications as read for a user
    bool markAllAsRead(int user_id);

    // Delete a notification
    bool deleteNotification(int notification_id);

    // Delete all notifications for a user
    bool deleteAllForUser(int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet

#endif // SOHBET_REPOSITORIES_NOTIFICATION_REPOSITORY_H
