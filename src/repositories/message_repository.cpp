#include "repositories/message_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

MessageRepository::MessageRepository(std::shared_ptr<db::Database> database)
    : database_(database) {
}

std::optional<Message> MessageRepository::createMessage(int conversation_id, int sender_id,
                                                        const std::string& content,
                                                        const std::string& media_url) {
    std::string query = "INSERT INTO messages (conversation_id, sender_id, content, media_url) "
                       "VALUES (?, ?, ?, ?)";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare create message query" << std::endl;
        return std::nullopt;
    }
    
    stmt.bindInt(1, conversation_id);
    stmt.bindInt(2, sender_id);
    stmt.bindText(3, content);
    
    if (media_url.empty()) {
        stmt.bindNull(4);
    } else {
        stmt.bindText(4, media_url);
    }
    
    if (stmt.step() == SQLITE_DONE) {
        int message_id = database_->lastInsertRowId();
        return getById(message_id);
    }
    
    std::cerr << "Failed to create message" << std::endl;
    return std::nullopt;
}

std::optional<Message> MessageRepository::getById(int id) {
    std::string query = "SELECT id, conversation_id, sender_id, content, media_url, "
                       "strftime('%s', read_at) as read_at, "
                       "strftime('%s', delivered_at) as delivered_at, "
                       "strftime('%s', created_at) as created_at "
                       "FROM messages WHERE id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return std::nullopt;
    }
    
    stmt.bindInt(1, id);
    
    if (stmt.step() == SQLITE_ROW) {
        Message msg;
        msg.id = stmt.getInt(0);
        msg.conversation_id = stmt.getInt(1);
        msg.sender_id = stmt.getInt(2);
        msg.content = stmt.getText(3);
        
        if (!stmt.isNull(4)) {
            msg.media_url = stmt.getText(4);
        }
        
        msg.is_read_at_null = stmt.isNull(5);
        if (!msg.is_read_at_null) {
            msg.read_at = stmt.getInt64(5);
        }
        
        msg.is_delivered_at_null = stmt.isNull(6);
        if (!msg.is_delivered_at_null) {
            msg.delivered_at = stmt.getInt64(6);
        }
        
        msg.created_at = stmt.getInt64(7);
        
        return msg;
    }
    
    return std::nullopt;
}

std::vector<Message> MessageRepository::getConversationMessages(int conversation_id, 
                                                                 int limit, int offset) {
    std::vector<Message> messages;
    
    std::string query = "SELECT id, conversation_id, sender_id, content, media_url, "
                       "strftime('%s', read_at) as read_at, "
                       "strftime('%s', delivered_at) as delivered_at, "
                       "strftime('%s', created_at) as created_at "
                       "FROM messages "
                       "WHERE conversation_id = ? "
                       "ORDER BY created_at DESC "
                       "LIMIT ? OFFSET ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare get conversation messages query" << std::endl;
        return messages;
    }
    
    stmt.bindInt(1, conversation_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);
    
    while (stmt.step() == SQLITE_ROW) {
        Message msg;
        msg.id = stmt.getInt(0);
        msg.conversation_id = stmt.getInt(1);
        msg.sender_id = stmt.getInt(2);
        msg.content = stmt.getText(3);
        
        if (!stmt.isNull(4)) {
            msg.media_url = stmt.getText(4);
        }
        
        msg.is_read_at_null = stmt.isNull(5);
        if (!msg.is_read_at_null) {
            msg.read_at = stmt.getInt64(5);
        }
        
        msg.is_delivered_at_null = stmt.isNull(6);
        if (!msg.is_delivered_at_null) {
            msg.delivered_at = stmt.getInt64(6);
        }
        
        msg.created_at = stmt.getInt64(7);
        
        messages.push_back(msg);
    }
    
    return messages;
}

bool MessageRepository::markAsDelivered(int message_id) {
    std::string query = "UPDATE messages SET delivered_at = CURRENT_TIMESTAMP "
                       "WHERE id = ? AND delivered_at IS NULL";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, message_id);
    return stmt.step() == SQLITE_DONE;
}

bool MessageRepository::markAsRead(int message_id) {
    std::string query = "UPDATE messages SET read_at = CURRENT_TIMESTAMP "
                       "WHERE id = ? AND read_at IS NULL";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, message_id);
    return stmt.step() == SQLITE_DONE;
}

bool MessageRepository::markAllAsRead(int conversation_id, int user_id) {
    std::string query = "UPDATE messages SET read_at = CURRENT_TIMESTAMP "
                       "WHERE conversation_id = ? AND sender_id != ? AND read_at IS NULL";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, conversation_id);
    stmt.bindInt(2, user_id);
    return stmt.step() == SQLITE_DONE;
}

int MessageRepository::getUnreadCount(int conversation_id, int user_id) {
    std::string query = "SELECT COUNT(*) FROM messages "
                       "WHERE conversation_id = ? AND sender_id != ? AND read_at IS NULL";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return 0;
    }
    
    stmt.bindInt(1, conversation_id);
    stmt.bindInt(2, user_id);
    
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    
    return 0;
}

bool MessageRepository::deleteMessage(int message_id) {
    std::string query = "DELETE FROM messages WHERE id = ?";
    
    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }
    
    stmt.bindInt(1, message_id);
    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
