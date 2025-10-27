#ifndef SOHBET_REPOSITORIES_CONVERSATION_REPOSITORY_H
#define SOHBET_REPOSITORIES_CONVERSATION_REPOSITORY_H

#include "db/database.h"
#include "models/conversation.h"
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

class ConversationRepository {
public:
    explicit ConversationRepository(std::shared_ptr<db::Database> database);

    // Find or create conversation between two users
    std::optional<Conversation> findOrCreateConversation(int user1_id, int user2_id);
    
    // Get a conversation by ID
    std::optional<Conversation> getById(int id);
    
    // Get all conversations for a user
    std::vector<Conversation> getUserConversations(int user_id);
    
    // Update last_message_at timestamp
    bool updateLastMessageTime(int conversation_id);
    
    // Delete a conversation
    bool deleteConversation(int conversation_id);

private:
    std::shared_ptr<db::Database> database_;
    
    std::optional<Conversation> findConversation(int user1_id, int user2_id);
    Conversation createConversation(int user1_id, int user2_id);
};

} // namespace repositories
} // namespace sohbet

#endif // SOHBET_REPOSITORIES_CONVERSATION_REPOSITORY_H
