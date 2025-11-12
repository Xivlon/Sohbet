#include "repositories/study_session_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

StudySessionRepository::StudySessionRepository(std::shared_ptr<db::Database> database)
    : database_(database) {
}

std::optional<StudySession> StudySessionRepository::createSession(
    int group_id, const std::string& title, const std::string& description,
    const std::string& location, std::optional<int> voice_channel_id,
    std::time_t start_time, std::time_t end_time, int created_by,
    std::optional<int> max_participants, bool is_recurring,
    const std::string& recurrence_pattern) {

    std::string query = "INSERT INTO study_sessions (group_id, title, description, location, "
                       "voice_channel_id, start_time, end_time, created_by, max_participants, "
                       "is_recurring, recurrence_pattern) "
                       "VALUES (?, ?, ?, ?, ?, to_timestamp(?), to_timestamp(?), ?, ?, ?, ?) RETURNING id";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        std::cerr << "Failed to prepare create session query" << std::endl;
        return std::nullopt;
    }

    stmt.bindInt(1, group_id);
    stmt.bindText(2, title);
    stmt.bindText(3, description);
    stmt.bindText(4, location);

    voice_channel_id.has_value() ? stmt.bindInt(5, voice_channel_id.value()) : stmt.bindNull(5);
    stmt.bindInt(6, static_cast<int>(start_time));
    stmt.bindInt(7, static_cast<int>(end_time));
    stmt.bindInt(8, created_by);
    max_participants.has_value() ? stmt.bindInt(9, max_participants.value()) : stmt.bindNull(9);
    stmt.bindInt(10, is_recurring ? 1 : 0);
    stmt.bindText(11, recurrence_pattern);

    if (stmt.step() == SQLITE_ROW) {
        int session_id = stmt.getInt(0);
        // Call step() again to commit the transaction
        stmt.step();
        return getById(session_id);
    }

    return std::nullopt;
}

std::optional<StudySession> StudySessionRepository::getById(int id) {
    std::string query = "SELECT id, group_id, title, description, location, voice_channel_id, "
                       "EXTRACT(EPOCH FROM start_time)::bigint as start_time, "
                       "EXTRACT(EPOCH FROM end_time)::bigint as end_time, "
                       "created_by, max_participants, is_recurring, recurrence_pattern, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM updated_at)::bigint as updated_at "
                       "FROM study_sessions WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return std::nullopt;
    }

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        StudySession session;
        session.id = stmt.getInt(0);
        session.group_id = stmt.getInt(1);
        session.title = stmt.getText(2);
        session.description = stmt.getText(3);
        session.location = stmt.getText(4);

        session.is_voice_channel_null = stmt.isNull(5);
        session.voice_channel_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));

        session.start_time = stmt.getInt64(6);
        session.end_time = stmt.getInt64(7);
        session.created_by = stmt.getInt(8);

        session.is_max_participants_null = stmt.isNull(9);
        session.max_participants = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        session.is_recurring = stmt.getInt(10) != 0;
        session.recurrence_pattern = stmt.getText(11);
        session.created_at = stmt.getInt64(12);
        session.updated_at = stmt.getInt64(13);

        return session;
    }

    return std::nullopt;
}

std::vector<StudySession> StudySessionRepository::getGroupSessions(int group_id) {
    std::vector<StudySession> sessions;

    std::string query = "SELECT id, group_id, title, description, location, voice_channel_id, "
                       "EXTRACT(EPOCH FROM start_time)::bigint as start_time, "
                       "EXTRACT(EPOCH FROM end_time)::bigint as end_time, "
                       "created_by, max_participants, is_recurring, recurrence_pattern, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM updated_at)::bigint as updated_at "
                       "FROM study_sessions "
                       "WHERE group_id = ? "
                       "ORDER BY start_time ASC";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return sessions;
    }

    stmt.bindInt(1, group_id);

    while (stmt.step() == SQLITE_ROW) {
        StudySession session;
        session.id = stmt.getInt(0);
        session.group_id = stmt.getInt(1);
        session.title = stmt.getText(2);
        session.description = stmt.getText(3);
        session.location = stmt.getText(4);

        session.is_voice_channel_null = stmt.isNull(5);
        session.voice_channel_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));

        session.start_time = stmt.getInt64(6);
        session.end_time = stmt.getInt64(7);
        session.created_by = stmt.getInt(8);

        session.is_max_participants_null = stmt.isNull(9);
        session.max_participants = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        session.is_recurring = stmt.getInt(10) != 0;
        session.recurrence_pattern = stmt.getText(11);
        session.created_at = stmt.getInt64(12);
        session.updated_at = stmt.getInt64(13);

        sessions.push_back(session);
    }

    return sessions;
}

std::vector<StudySession> StudySessionRepository::getUpcomingSessions(int group_id, int limit) {
    std::vector<StudySession> sessions;

    std::string query = "SELECT id, group_id, title, description, location, voice_channel_id, "
                       "EXTRACT(EPOCH FROM start_time)::bigint as start_time, "
                       "EXTRACT(EPOCH FROM end_time)::bigint as end_time, "
                       "created_by, max_participants, is_recurring, recurrence_pattern, "
                       "EXTRACT(EPOCH FROM created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM updated_at)::bigint as updated_at "
                       "FROM study_sessions "
                       "WHERE group_id = ? AND start_time >= CURRENT_TIMESTAMP "
                       "ORDER BY start_time ASC "
                       "LIMIT ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return sessions;
    }

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, limit);

    while (stmt.step() == SQLITE_ROW) {
        StudySession session;
        session.id = stmt.getInt(0);
        session.group_id = stmt.getInt(1);
        session.title = stmt.getText(2);
        session.description = stmt.getText(3);
        session.location = stmt.getText(4);

        session.is_voice_channel_null = stmt.isNull(5);
        session.voice_channel_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));

        session.start_time = stmt.getInt64(6);
        session.end_time = stmt.getInt64(7);
        session.created_by = stmt.getInt(8);

        session.is_max_participants_null = stmt.isNull(9);
        session.max_participants = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        session.is_recurring = stmt.getInt(10) != 0;
        session.recurrence_pattern = stmt.getText(11);
        session.created_at = stmt.getInt64(12);
        session.updated_at = stmt.getInt64(13);

        sessions.push_back(session);
    }

    return sessions;
}

std::vector<StudySession> StudySessionRepository::getUserSessions(int user_id) {
    std::vector<StudySession> sessions;

    std::string query = "SELECT s.id, s.group_id, s.title, s.description, s.location, "
                       "s.voice_channel_id, "
                       "EXTRACT(EPOCH FROM s.start_time)::bigint as start_time, "
                       "EXTRACT(EPOCH FROM s.end_time)::bigint as end_time, "
                       "s.created_by, s.max_participants, s.is_recurring, s.recurrence_pattern, "
                       "EXTRACT(EPOCH FROM s.created_at)::bigint as created_at, "
                       "EXTRACT(EPOCH FROM s.updated_at)::bigint as updated_at "
                       "FROM study_sessions s "
                       "INNER JOIN session_participants sp ON s.id = sp.session_id "
                       "WHERE sp.user_id = ? AND s.start_time >= CURRENT_TIMESTAMP "
                       "ORDER BY s.start_time ASC";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return sessions;
    }

    stmt.bindInt(1, user_id);

    while (stmt.step() == SQLITE_ROW) {
        StudySession session;
        session.id = stmt.getInt(0);
        session.group_id = stmt.getInt(1);
        session.title = stmt.getText(2);
        session.description = stmt.getText(3);
        session.location = stmt.getText(4);

        session.is_voice_channel_null = stmt.isNull(5);
        session.voice_channel_id = stmt.isNull(5) ? std::nullopt : std::optional<int>(stmt.getInt(5));

        session.start_time = stmt.getInt64(6);
        session.end_time = stmt.getInt64(7);
        session.created_by = stmt.getInt(8);

        session.is_max_participants_null = stmt.isNull(9);
        session.max_participants = stmt.isNull(9) ? std::nullopt : std::optional<int>(stmt.getInt(9));

        session.is_recurring = stmt.getInt(10) != 0;
        session.recurrence_pattern = stmt.getText(11);
        session.created_at = stmt.getInt64(12);
        session.updated_at = stmt.getInt64(13);

        sessions.push_back(session);
    }

    return sessions;
}

bool StudySessionRepository::updateSession(int session_id, const std::string& title,
                                           const std::string& description, const std::string& location,
                                           std::time_t start_time, std::time_t end_time) {
    std::string query = "UPDATE study_sessions SET title = ?, description = ?, location = ?, "
                       "start_time = to_timestamp(?), end_time = to_timestamp(?), "
                       "updated_at = CURRENT_TIMESTAMP WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindText(1, title);
    stmt.bindText(2, description);
    stmt.bindText(3, location);
    stmt.bindInt(4, static_cast<int>(start_time));
    stmt.bindInt(5, static_cast<int>(end_time));
    stmt.bindInt(6, session_id);

    return stmt.step() == SQLITE_DONE;
}

bool StudySessionRepository::deleteSession(int session_id) {
    std::string query = "DELETE FROM study_sessions WHERE id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, session_id);

    return stmt.step() == SQLITE_DONE;
}

bool StudySessionRepository::addParticipant(int session_id, int user_id, const std::string& status) {
    std::string query = "INSERT INTO session_participants (session_id, user_id, status) "
                       "VALUES (?, ?, ?) "
                       "ON CONFLICT(session_id, user_id) DO UPDATE SET status = excluded.status";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, session_id);
    stmt.bindInt(2, user_id);
    stmt.bindText(3, status);

    return stmt.step() == SQLITE_DONE;
}

bool StudySessionRepository::removeParticipant(int session_id, int user_id) {
    std::string query = "DELETE FROM session_participants WHERE session_id = ? AND user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, session_id);
    stmt.bindInt(2, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool StudySessionRepository::updateParticipantStatus(int session_id, int user_id, const std::string& status) {
    std::string query = "UPDATE session_participants SET status = ? WHERE session_id = ? AND user_id = ?";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindText(1, status);
    stmt.bindInt(2, session_id);
    stmt.bindInt(3, user_id);

    return stmt.step() == SQLITE_DONE;
}

int StudySessionRepository::getParticipantCount(int session_id) {
    std::string query = "SELECT COUNT(*) FROM session_participants WHERE session_id = ? AND status = 'going'";

    db::Statement stmt(*database_, query);
    if (!stmt.isValid()) {
        return 0;
    }

    stmt.bindInt(1, session_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

} // namespace repositories
} // namespace sohbet
