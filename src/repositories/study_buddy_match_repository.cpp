#include "repositories/study_buddy_match_repository.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sohbet {
namespace repositories {

StudyBuddyMatchRepository::StudyBuddyMatchRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<StudyBuddyMatch> StudyBuddyMatchRepository::findById(int matchId) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, matched_user_id, compatibility_score, course_overlap_score,
               schedule_compatibility_score, learning_style_score, academic_level_score,
               common_courses, common_interests, match_reason, status,
               viewed_at, responded_at, created_at, updated_at
        FROM study_buddy_matches
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, matchId);

    if (stmt.step() == SQLITE_ROW) {
        return buildFromRow(stmt);
    }

    return std::nullopt;
}

std::optional<StudyBuddyMatch> StudyBuddyMatchRepository::create(const StudyBuddyMatch& match) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    json coursesJson = json::array();
    for (const auto& course : match.common_courses) {
        coursesJson.push_back(course);
    }

    json interestsJson = json::array();
    for (const auto& interest : match.common_interests) {
        interestsJson.push_back(interest);
    }

    const std::string sql = R"(
        INSERT INTO study_buddy_matches (
            user_id, matched_user_id, compatibility_score, course_overlap_score,
            schedule_compatibility_score, learning_style_score, academic_level_score,
            common_courses, common_interests, match_reason, status
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, match.user_id);
    stmt.bindInt(2, match.matched_user_id);
    stmt.bindDouble(3, match.compatibility_score);
    stmt.bindDouble(4, match.course_overlap_score);
    stmt.bindDouble(5, match.schedule_compatibility_score);
    stmt.bindDouble(6, match.learning_style_score);
    stmt.bindDouble(7, match.academic_level_score);
    stmt.bindText(8, coursesJson.dump());
    stmt.bindText(9, interestsJson.dump());
    stmt.bindText(10, match.match_reason);
    stmt.bindText(11, StudyBuddyMatch::matchStatusToString(match.status));

    if (stmt.step() == SQLITE_DONE) {
        StudyBuddyMatch created = match;
        created.id = static_cast<int>(database_->lastInsertRowId());
        return created;
    }

    return std::nullopt;
}

std::vector<StudyBuddyMatch> StudyBuddyMatchRepository::findByUserId(
    int userId,
    std::optional<MatchStatus> status,
    int limit
) {
    std::vector<StudyBuddyMatch> result;
    if (!database_ || !database_->isOpen()) return result;

    std::string sql = R"(
        SELECT id, user_id, matched_user_id, compatibility_score, course_overlap_score,
               schedule_compatibility_score, learning_style_score, academic_level_score,
               common_courses, common_interests, match_reason, status,
               viewed_at, responded_at, created_at, updated_at
        FROM study_buddy_matches
        WHERE user_id = ?
    )";

    if (status.has_value()) {
        sql += " AND status = ?";
    }

    sql += " ORDER BY compatibility_score DESC LIMIT ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return result;

    int paramIndex = 1;
    stmt.bindInt(paramIndex++, userId);
    if (status.has_value()) {
        stmt.bindText(paramIndex++, StudyBuddyMatch::matchStatusToString(status.value()));
    }
    stmt.bindInt(paramIndex, limit);

    while (stmt.step() == SQLITE_ROW) {
        result.push_back(buildFromRow(stmt));
    }

    return result;
}

std::vector<StudyBuddyMatch> StudyBuddyMatchRepository::findSuggestedMatches(int userId, int limit) {
    return findByUserId(userId, MatchStatus::SUGGESTED, limit);
}

std::vector<StudyBuddyMatch> StudyBuddyMatchRepository::findAcceptedMatches(int userId) {
    return findByUserId(userId, MatchStatus::ACCEPTED, 100);
}

bool StudyBuddyMatchRepository::updateStatus(int matchId, MatchStatus status) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE study_buddy_matches
        SET status = ?, responded_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, StudyBuddyMatch::matchStatusToString(status));
    stmt.bindInt(2, matchId);

    return stmt.step() == SQLITE_DONE;
}

bool StudyBuddyMatchRepository::deleteById(int matchId) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM study_buddy_matches WHERE id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, matchId);
    return stmt.step() == SQLITE_DONE;
}

StudyBuddyMatch StudyBuddyMatchRepository::buildFromRow(db::Statement& stmt) {
    StudyBuddyMatch match;

    match.id = stmt.getInt(0);
    match.user_id = stmt.getInt(1);
    match.matched_user_id = stmt.getInt(2);
    match.compatibility_score = stmt.getDouble(3);
    match.course_overlap_score = stmt.getDouble(4);
    match.schedule_compatibility_score = stmt.getDouble(5);
    match.learning_style_score = stmt.getDouble(6);
    match.academic_level_score = stmt.getDouble(7);

    std::string coursesStr = stmt.getText(8);
    if (!coursesStr.empty()) {
        try {
            json coursesJson = json::parse(coursesStr);
            if (coursesJson.is_array()) {
                for (const auto& course : coursesJson) {
                    match.common_courses.push_back(course.get<std::string>());
                }
            }
        } catch (...) {}
    }

    std::string interestsStr = stmt.getText(9);
    if (!interestsStr.empty()) {
        try {
            json interestsJson = json::parse(interestsStr);
            if (interestsJson.is_array()) {
                for (const auto& interest : interestsJson) {
                    match.common_interests.push_back(interest.get<std::string>());
                }
            }
        } catch (...) {}
    }

    match.match_reason = stmt.getText(10);

    std::string statusStr = stmt.getText(11);
    match.status = !statusStr.empty() ? StudyBuddyMatch::stringToMatchStatus(statusStr) : MatchStatus::SUGGESTED;

    match.viewed_at = stmt.getInt64(12);
    match.responded_at = stmt.getInt64(13);
    match.created_at = stmt.getInt64(14);
    match.updated_at = stmt.getInt64(15);

    return match;
}

} // namespace repositories
} // namespace sohbet
