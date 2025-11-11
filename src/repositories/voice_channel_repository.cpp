#include "repositories/voice_channel_repository.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace sohbet {
namespace repositories {

VoiceChannelRepository::VoiceChannelRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<VoiceChannel> VoiceChannelRepository::create(VoiceChannel& channel) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO voice_channels (name, channel_type, group_id, organization_id, murmur_channel_id)
        VALUES (?, ?, ?, ?, ?)
        RETURNING id, created_at
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, channel.name);
    stmt.bindText(2, channel.channel_type);
    
    if (channel.group_id > 0) {
        stmt.bindInt(3, channel.group_id);
    } else {
        stmt.bindNull(3);
    }
    
    if (channel.organization_id > 0) {
        stmt.bindInt(4, channel.organization_id);
    } else {
        stmt.bindNull(4);
    }
    
    if (!channel.murmur_channel_id.empty()) {
        stmt.bindText(5, channel.murmur_channel_id);
    } else {
        stmt.bindNull(5);
    }

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        // PostgreSQL returns the inserted row with RETURNING clause
        channel.id = stmt.getInt(0);

        // Parse created_at timestamp
        std::string created_at_str = stmt.getText(1);
        std::tm tm = {};
        std::istringstream ss(created_at_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (!ss.fail()) {
            channel.created_at = std::mktime(&tm);
        } else {
            channel.created_at = std::time(nullptr);
        }

        return channel;
    }

    return std::nullopt;
}

std::optional<VoiceChannel> VoiceChannelRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, channel_type, group_id, organization_id, murmur_channel_id, created_at
        FROM voice_channels WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        VoiceChannel channel;
        channel.id = stmt.getInt(0);
        channel.name = stmt.getText(1);
        channel.channel_type = stmt.getText(2);
        
        if (!stmt.isNull(3)) {
            channel.group_id = stmt.getInt(3);
        }
        
        if (!stmt.isNull(4)) {
            channel.organization_id = stmt.getInt(4);
        }
        
        if (!stmt.isNull(5)) {
            channel.murmur_channel_id = stmt.getText(5);
        }
        
        // Parse created_at timestamp
        std::string created_at_str = stmt.getText(6);
        std::tm tm = {};
        std::istringstream ss(created_at_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (!ss.fail()) {
            channel.created_at = std::mktime(&tm);
        }
        
        return channel;
    }

    return std::nullopt;
}

std::vector<VoiceChannel> VoiceChannelRepository::findAll(int limit, int offset) {
    std::vector<VoiceChannel> channels;
    if (!database_ || !database_->isOpen()) return channels;

    const std::string sql = R"(
        SELECT id, name, channel_type, group_id, organization_id, murmur_channel_id, created_at
        FROM voice_channels
        ORDER BY created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return channels;

    stmt.bindInt(1, limit);
    stmt.bindInt(2, offset);

    while (stmt.step() == SQLITE_ROW) {
        VoiceChannel channel;
        channel.id = stmt.getInt(0);
        channel.name = stmt.getText(1);
        channel.channel_type = stmt.getText(2);
        
        if (!stmt.isNull(3)) {
            channel.group_id = stmt.getInt(3);
        }
        
        if (!stmt.isNull(4)) {
            channel.organization_id = stmt.getInt(4);
        }
        
        if (!stmt.isNull(5)) {
            channel.murmur_channel_id = stmt.getText(5);
        }
        
        // Parse created_at timestamp
        std::string created_at_str = stmt.getText(6);
        std::tm tm = {};
        std::istringstream ss(created_at_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (!ss.fail()) {
            channel.created_at = std::mktime(&tm);
        }
        
        channels.push_back(channel);
    }

    return channels;
}

std::vector<VoiceChannel> VoiceChannelRepository::findByType(const std::string& channel_type, int limit, int offset) {
    std::vector<VoiceChannel> channels;
    if (!database_ || !database_->isOpen()) return channels;

    const std::string sql = R"(
        SELECT id, name, channel_type, group_id, organization_id, murmur_channel_id, created_at
        FROM voice_channels
        WHERE channel_type = ?
        ORDER BY created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return channels;

    stmt.bindText(1, channel_type);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        VoiceChannel channel;
        channel.id = stmt.getInt(0);
        channel.name = stmt.getText(1);
        channel.channel_type = stmt.getText(2);
        
        if (!stmt.isNull(3)) {
            channel.group_id = stmt.getInt(3);
        }
        
        if (!stmt.isNull(4)) {
            channel.organization_id = stmt.getInt(4);
        }
        
        if (!stmt.isNull(5)) {
            channel.murmur_channel_id = stmt.getText(5);
        }
        
        // Parse created_at timestamp
        std::string created_at_str = stmt.getText(6);
        std::tm tm = {};
        std::istringstream ss(created_at_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (!ss.fail()) {
            channel.created_at = std::mktime(&tm);
        }
        
        channels.push_back(channel);
    }

    return channels;
}

bool VoiceChannelRepository::updateMurmurChannelId(int id, const std::string& murmur_channel_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE voice_channels
        SET murmur_channel_id = ?
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, murmur_channel_id);
    stmt.bindInt(2, id);

    return stmt.step() == SQLITE_DONE;
}

bool VoiceChannelRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM voice_channels WHERE id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);

    return stmt.step() == SQLITE_DONE;
}

int VoiceChannelRepository::createSession(int channel_id, int user_id, const std::string& murmur_session_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        INSERT INTO voice_sessions (channel_id, user_id, murmur_session_id)
        VALUES (?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, channel_id);
    stmt.bindInt(2, user_id);
    
    if (!murmur_session_id.empty()) {
        stmt.bindText(3, murmur_session_id);
    } else {
        stmt.bindNull(3);
    }

    if (stmt.step() == SQLITE_DONE) {
        return static_cast<int>(database_->lastInsertRowId());
    }

    return 0;
}

bool VoiceChannelRepository::endSession(int session_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE voice_sessions
        SET left_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, session_id);

    return stmt.step() == SQLITE_DONE;
}

int VoiceChannelRepository::getActiveUserCount(int channel_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        SELECT COUNT(*) FROM voice_sessions
        WHERE channel_id = ? AND left_at IS NULL
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, channel_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

int VoiceChannelRepository::getUserActiveSession(int user_id, int channel_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        SELECT id FROM voice_sessions
        WHERE user_id = ? AND channel_id = ? AND left_at IS NULL
        ORDER BY joined_at DESC
        LIMIT 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, channel_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

int VoiceChannelRepository::endAllUserSessions(int user_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        UPDATE voice_sessions
        SET left_at = CURRENT_TIMESTAMP
        WHERE user_id = ? AND left_at IS NULL
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, user_id);

    if (stmt.step() == SQLITE_DONE) {
        // Get number of rows changed
        return static_cast<int>(stmt.affectedRows());
    }

    return 0;
}

std::vector<int> VoiceChannelRepository::findEmptyInactiveChannels(int inactivity_minutes) {
    std::vector<int> channel_ids;
    if (!database_ || !database_->isOpen()) return channel_ids;

    // Find channels that:
    // 1. Have at least one session (have been used before)
    // 2. Have no active sessions (all sessions have left_at set)
    // 3. Last session ended more than inactivity_minutes ago
    const std::string sql = R"(
        SELECT vc.id
        FROM voice_channels vc
        INNER JOIN (
            SELECT channel_id, MAX(left_at) as last_activity
            FROM voice_sessions
            GROUP BY channel_id
        ) vs ON vc.id = vs.channel_id
        WHERE vs.last_activity IS NOT NULL
        AND vs.last_activity < datetime('now', '-' || ? || ' minutes')
        AND NOT EXISTS (
            SELECT 1 FROM voice_sessions
            WHERE channel_id = vc.id AND left_at IS NULL
        )
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return channel_ids;

    stmt.bindInt(1, inactivity_minutes);

    while (stmt.step() == SQLITE_ROW) {
        channel_ids.push_back(stmt.getInt(0));
    }

    return channel_ids;
}

} // namespace repositories
} // namespace sohbet
