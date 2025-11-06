#include "repositories/study_buddy_connection_repository.h"

namespace sohbet {
namespace repositories {

StudyBuddyConnectionRepository::StudyBuddyConnectionRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<StudyBuddyConnection> StudyBuddyConnectionRepository::create(const StudyBuddyConnection& connection) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO study_buddy_connections (
            user_id, buddy_id, total_study_sessions, connection_strength,
            is_favorite, notification_enabled, notes
        ) VALUES (?, ?, ?, ?, ?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, connection.user_id);
    stmt.bindInt(2, connection.buddy_id);
    stmt.bindInt(3, connection.total_study_sessions);
    stmt.bindInt(4, connection.connection_strength);
    stmt.bindInt(5, connection.is_favorite ? 1 : 0);
    stmt.bindInt(6, connection.notification_enabled ? 1 : 0);
    stmt.bindText(7, connection.notes);

    if (stmt.step() == SQLITE_DONE) {
        StudyBuddyConnection created = connection;
        created.id = static_cast<int>(database_->lastInsertRowId());
        return created;
    }

    return std::nullopt;
}

std::vector<StudyBuddyConnection> StudyBuddyConnectionRepository::findByUserId(int userId) {
    std::vector<StudyBuddyConnection> result;
    if (!database_ || !database_->isOpen()) return result;

    const std::string sql = R"(
        SELECT id, user_id, buddy_id, connected_at, last_study_session,
               total_study_sessions, connection_strength, is_favorite,
               notification_enabled, notes, created_at, updated_at
        FROM study_buddy_connections
        WHERE user_id = ?
        ORDER BY connection_strength DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return result;

    stmt.bindInt(1, userId);

    while (stmt.step() == SQLITE_ROW) {
        result.push_back(buildFromRow(stmt.getStatement()));
    }

    return result;
}

std::optional<StudyBuddyConnection> StudyBuddyConnectionRepository::findConnectionBetweenUsers(int userId1, int userId2) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, buddy_id, connected_at, last_study_session,
               total_study_sessions, connection_strength, is_favorite,
               notification_enabled, notes, created_at, updated_at
        FROM study_buddy_connections
        WHERE (user_id = ? AND buddy_id = ?) OR (user_id = ? AND buddy_id = ?)
        LIMIT 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, userId1);
    stmt.bindInt(2, userId2);
    stmt.bindInt(3, userId2);
    stmt.bindInt(4, userId1);

    if (stmt.step() == SQLITE_ROW) {
        return buildFromRow(stmt.getStatement());
    }

    return std::nullopt;
}

StudyBuddyConnection StudyBuddyConnectionRepository::buildFromRow(sqlite3_stmt* stmt) {
    StudyBuddyConnection conn;

    conn.id = sqlite3_column_int(stmt, 0);
    conn.user_id = sqlite3_column_int(stmt, 1);
    conn.buddy_id = sqlite3_column_int(stmt, 2);
    conn.connected_at = sqlite3_column_int64(stmt, 3);
    conn.last_study_session = sqlite3_column_int64(stmt, 4);
    conn.total_study_sessions = sqlite3_column_int(stmt, 5);
    conn.connection_strength = sqlite3_column_int(stmt, 6);
    conn.is_favorite = sqlite3_column_int(stmt, 7) == 1;
    conn.notification_enabled = sqlite3_column_int(stmt, 8) == 1;

    const char* notesStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
    conn.notes = notesStr ? notesStr : "";

    conn.created_at = sqlite3_column_int64(stmt, 10);
    conn.updated_at = sqlite3_column_int64(stmt, 11);

    return conn;
}

} // namespace repositories
} // namespace sohbet
