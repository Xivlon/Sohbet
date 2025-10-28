#include "repositories/conversation_repository.h"
#include <algorithm>
#include <iostream>

namespace sohbet {
namespace repositories {

ConversationRepository::ConversationRepository(std::shared_ptr<db::Database> database)
    : database_(database) {
}

std::optional<Conversation> ConversationRepository::findOrCreateConversation(int user1_id, int user2_id) {
    // Ensure user1_id < user2_id for consistent ordering
    if (user1_id > user2_id) {
        std::swap(user1_id, user2_id);
    }
    
    // Try to find existing conversation
    auto existing = findConversation(user1_id, user2_id);
    if (existing) {
        return existing;
    }
    
    // Create new conversation
    return createConversation(user1_id, user2_id);
}

std::optional<Conversation> ConversationRepository::findConversation(int user1_id, int user2_id) {
    std::string query = "SELECT id, user1_id, user2_id, "
                       "strftime('%s', created_at) as created_at, "
                       "strftime('%s', last_message_at) as last_message_at "
                       "FROM conversations "
                       "WHERE user1_id = ? AND user2_id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare find conversation query" << std::endl;
        return std::nullopt;
    }
    
    stmt.bindInt(1, user1_id);
    stmt.bindInt(2, user2_id);
    
    if (stmt.step() == SQLITE_ROW) {
        Conversation conv;
        conv.id = stmt.getInt(0);
        conv.user1_id = stmt.getInt(1);
        conv.user2_id = stmt.getInt(2);
        conv.created_at = stmt.getInt64(3);
        conv.last_message_at = stmt.getInt64(4);
        return conv;
    }
    
    return std::nullopt;
}

Conversation ConversationRepository::createConversation(int user1_id, int user2_id) {
    std::string query = "INSERT INTO conversations (user1_id, user2_id) VALUES (?, ?)";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare create conversation query" << std::endl;
        return Conversation();
    }
    
    stmt.bindInt(1, user1_id);
    stmt.bindInt(2, user2_id);
    
    if (stmt.step() == SQLITE_DONE) {
        // Conversation created successfully
        // Retrieve the created conversation
        auto created = findConversation(user1_id, user2_id);
        if (created) {
            return *created;
        }
    }
    
    std::cerr << "Failed to create conversation" << std::endl;
    return Conversation();
}

std::optional<Conversation> ConversationRepository::getById(int id) {
    std::string query = "SELECT id, user1_id, user2_id, "
                       "strftime('%s', created_at) as created_at, "
                       "strftime('%s', last_message_at) as last_message_at "
                       "FROM conversations WHERE id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return std::nullopt;
    }
    
    stmt.bindInt(1, id);
    
    if (stmt.step() == SQLITE_ROW) {
        Conversation conv;
        conv.id = stmt.getInt(0);
        conv.user1_id = stmt.getInt(1);
        conv.user2_id = stmt.getInt(2);
        conv.created_at = stmt.getInt64(3);
        conv.last_message_at = stmt.getInt64(4);
        return conv;
    }
    
    return std::nullopt;
}

std::vector<Conversation> ConversationRepository::getUserConversations(int user_id) {
    std::vector<Conversation> conversations;
    
    std::string query = "SELECT id, user1_id, user2_id, "
                       "strftime('%s', created_at) as created_at, "
                       "strftime('%s', last_message_at) as last_message_at "
                       "FROM conversations "
                       "WHERE user1_id = ? OR user2_id = ? "
                       "ORDER BY last_message_at DESC";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare get user conversations query" << std::endl;
        return conversations;
    }
    
    stmt.bindInt(1, user_id);
    stmt.bindInt(2, user_id);
    
    while (stmt.step() == SQLITE_ROW) {
        Conversation conv;
        conv.id = stmt.getInt(0);
        conv.user1_id = stmt.getInt(1);
        conv.user2_id = stmt.getInt(2);
        conv.created_at = stmt.getInt64(3);
        conv.last_message_at = stmt.getInt64(4);
        conversations.push_back(conv);
    }
    
    return conversations;
}

bool ConversationRepository::updateLastMessageTime(int conversation_id) {
    std::string query = "UPDATE conversations SET last_message_at = CURRENT_TIMESTAMP "
                       "WHERE id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, conversation_id);
    return stmt.step() == SQLITE_DONE;
}

bool ConversationRepository::deleteConversation(int conversation_id) {
    std::string query = "DELETE FROM conversations WHERE id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, conversation_id);
    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
