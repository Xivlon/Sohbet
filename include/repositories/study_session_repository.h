#ifndef SOHBET_REPOSITORIES_STUDY_SESSION_REPOSITORY_H
#define SOHBET_REPOSITORIES_STUDY_SESSION_REPOSITORY_H

#include "db/database.h"
#include "models/study_session.h"
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace repositories {

class StudySessionRepository {
public:
    explicit StudySessionRepository(std::shared_ptr<db::Database> database);

    // Create a new study session
    std::optional<StudySession> createSession(int group_id, const std::string& title,
                                              const std::string& description,
                                              const std::string& location,
                                              std::optional<int> voice_channel_id,
                                              std::time_t start_time, std::time_t end_time,
                                              int created_by,
                                              std::optional<int> max_participants = std::nullopt,
                                              bool is_recurring = false,
                                              const std::string& recurrence_pattern = "");

    // Get session by ID
    std::optional<StudySession> getById(int id);

    // Get all sessions for a group
    std::vector<StudySession> getGroupSessions(int group_id);

    // Get upcoming sessions for a group
    std::vector<StudySession> getUpcomingSessions(int group_id, int limit = 10);

    // Get sessions user is participating in
    std::vector<StudySession> getUserSessions(int user_id);

    // Update session
    bool updateSession(int session_id, const std::string& title,
                      const std::string& description, const std::string& location,
                      std::time_t start_time, std::time_t end_time);

    // Delete session
    bool deleteSession(int session_id);

    // Add participant to session
    bool addParticipant(int session_id, int user_id, const std::string& status = "going");

    // Remove participant from session
    bool removeParticipant(int session_id, int user_id);

    // Update participant status
    bool updateParticipantStatus(int session_id, int user_id, const std::string& status);

    // Get participant count
    int getParticipantCount(int session_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet

#endif // SOHBET_REPOSITORIES_STUDY_SESSION_REPOSITORY_H
