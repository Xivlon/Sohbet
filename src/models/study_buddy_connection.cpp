#include "models/study_buddy_connection.h"
#include <cmath>
#include <algorithm>

StudyBuddyConnection::StudyBuddyConnection()
    : id(0),
      user_id(0),
      buddy_id(0),
      connected_at(std::time(nullptr)),
      last_study_session(0),
      total_study_sessions(0),
      connection_strength(0),
      is_favorite(false),
      notification_enabled(true),
      created_at(std::time(nullptr)),
      updated_at(std::time(nullptr)) {}

json StudyBuddyConnection::toJson() const {
    json result = {
        {"id", id},
        {"user_id", user_id},
        {"buddy_id", buddy_id},
        {"connected_at", connected_at},
        {"total_study_sessions", total_study_sessions},
        {"connection_strength", connection_strength},
        {"is_favorite", is_favorite},
        {"notification_enabled", notification_enabled},
        {"notes", notes},
        {"created_at", created_at},
        {"updated_at", updated_at}
    };

    if (last_study_session > 0) {
        result["last_study_session"] = last_study_session;
    }

    return result;
}

StudyBuddyConnection StudyBuddyConnection::fromJson(const json& j) {
    StudyBuddyConnection conn;
    conn.id = j.value("id", 0);
    conn.user_id = j.value("user_id", 0);
    conn.buddy_id = j.value("buddy_id", 0);
    conn.connected_at = j.value("connected_at", std::time(nullptr));
    conn.last_study_session = j.value("last_study_session", 0);
    conn.total_study_sessions = j.value("total_study_sessions", 0);
    conn.connection_strength = j.value("connection_strength", 0);
    conn.is_favorite = j.value("is_favorite", false);
    conn.notification_enabled = j.value("notification_enabled", true);
    conn.notes = j.value("notes", "");
    conn.created_at = j.value("created_at", std::time(nullptr));
    conn.updated_at = j.value("updated_at", std::time(nullptr));

    return conn;
}

void StudyBuddyConnection::incrementStudySessions() {
    total_study_sessions++;
    last_study_session = std::time(nullptr);
    updateConnectionStrength();
}

void StudyBuddyConnection::updateConnectionStrength() {
    // Calculate connection strength based on:
    // 1. Total number of study sessions (40%)
    // 2. Recency of interactions (30%)
    // 3. Consistency/frequency (30%)

    // Session count score (0-40 points)
    double sessionScore = std::min(40.0, total_study_sessions * 4.0);

    // Recency score (0-30 points)
    double recencyScore = 0.0;
    if (last_study_session > 0) {
        std::time_t now = std::time(nullptr);
        double daysSinceLastSession = std::difftime(now, last_study_session) / (60 * 60 * 24);

        if (daysSinceLastSession <= 7) {
            recencyScore = 30.0;
        } else if (daysSinceLastSession <= 14) {
            recencyScore = 25.0;
        } else if (daysSinceLastSession <= 30) {
            recencyScore = 20.0;
        } else if (daysSinceLastSession <= 60) {
            recencyScore = 10.0;
        } else {
            recencyScore = 5.0;
        }
    }

    // Frequency score (0-30 points)
    double frequencyScore = 0.0;
    if (total_study_sessions > 0 && connected_at > 0) {
        std::time_t now = std::time(nullptr);
        double daysSinceConnection = std::difftime(now, connected_at) / (60 * 60 * 24);

        if (daysSinceConnection > 0) {
            double sessionsPerWeek = (total_study_sessions * 7.0) / daysSinceConnection;
            frequencyScore = std::min(30.0, sessionsPerWeek * 15.0);
        }
    }

    connection_strength = static_cast<int>(sessionScore + recencyScore + frequencyScore);
    connection_strength = std::max(0, std::min(100, connection_strength));
}
