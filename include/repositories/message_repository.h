#ifndef SOHBET_REPOSITORIES_MESSAGE_REPOSITORY_H
#define SOHBET_REPOSITORIES_MESSAGE_REPOSITORY_H

#include "db/database.h"
#include "models/message.h"
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

class MessageRepository {
public:
    explicit MessageRepository(std::shared_ptr<db::Database> database);

    // Create a new message
    std::optional<Message> createMessage(int conversation_id, int sender_id, 
                                         const std::string& content, 
                                         const std::string& media_url = "");
    
    // Get a message by ID
    std::optional<Message> getById(int id);
    
    // Get messages for a conversation with pagination
    std::vector<Message> getConversationMessages(int conversation_id, int limit = 50, int offset = 0);
    
    // Mark message as delivered
    bool markAsDelivered(int message_id);
    
    // Mark message as read
    bool markAsRead(int message_id);
    
    // Mark all messages in a conversation as read
    bool markAllAsRead(int conversation_id, int user_id);
    
    // Get unread message count for a user in a conversation
    int getUnreadCount(int conversation_id, int user_id);
    
    // Delete a message
    bool deleteMessage(int message_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet

#endif // SOHBET_REPOSITORIES_MESSAGE_REPOSITORY_H
