#include "repositories/study_preferences_repository.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sohbet {
namespace repositories {

StudyPreferencesRepository::StudyPreferencesRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<StudyPreferences> StudyPreferencesRepository::upsert(const StudyPreferences& prefs) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    // Convert vectors to JSON strings
    json coursesJson = json::array();
    for (const auto& course : prefs.courses) {
        coursesJson.push_back(course);
    }

    json topicsJson = json::array();
    for (const auto& topic : prefs.topics_of_interest) {
        topicsJson.push_back(topic);
    }

    json daysJson = json::array();
    for (const auto& day : prefs.available_days) {
        daysJson.push_back(day);
    }

    const std::string sql = R"(
        INSERT INTO study_preferences (
            user_id, learning_style, study_environment, study_time_preference,
            courses, topics_of_interest, academic_goals, available_days,
            available_hours_per_week, preferred_group_size, same_university_only,
            same_department_only, same_year_only, is_active
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(user_id) DO UPDATE SET
            learning_style = excluded.learning_style,
            study_environment = excluded.study_environment,
            study_time_preference = excluded.study_time_preference,
            courses = excluded.courses,
            topics_of_interest = excluded.topics_of_interest,
            academic_goals = excluded.academic_goals,
            available_days = excluded.available_days,
            available_hours_per_week = excluded.available_hours_per_week,
            preferred_group_size = excluded.preferred_group_size,
            same_university_only = excluded.same_university_only,
            same_department_only = excluded.same_department_only,
            same_year_only = excluded.same_year_only,
            is_active = excluded.is_active,
            updated_at = CURRENT_TIMESTAMP
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, prefs.user_id);
    stmt.bindText(2, StudyPreferences::learningStyleToString(prefs.learning_style));
    stmt.bindText(3, StudyPreferences::studyEnvironmentToString(prefs.study_environment));
    stmt.bindText(4, StudyPreferences::studyTimePreferenceToString(prefs.study_time_preference));
    stmt.bindText(5, coursesJson.dump());
    stmt.bindText(6, topicsJson.dump());
    stmt.bindText(7, prefs.academic_goals);
    stmt.bindText(8, daysJson.dump());
    stmt.bindInt(9, prefs.available_hours_per_week);
    stmt.bindInt(10, prefs.preferred_group_size);
    stmt.bindInt(11, prefs.same_university_only ? 1 : 0);
    stmt.bindInt(12, prefs.same_department_only ? 1 : 0);
    stmt.bindInt(13, prefs.same_year_only ? 1 : 0);
    stmt.bindInt(14, prefs.is_active ? 1 : 0);

    if (stmt.step() == SQLITE_DONE) {
        return findByUserId(prefs.user_id);
    }

    return std::nullopt;
}

std::optional<StudyPreferences> StudyPreferencesRepository::findByUserId(int userId) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, learning_style, study_environment, study_time_preference,
               courses, topics_of_interest, academic_goals, available_days,
               available_hours_per_week, preferred_group_size, same_university_only,
               same_department_only, same_year_only, is_active, created_at, updated_at
        FROM study_preferences WHERE user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, userId);

    if (stmt.step() == SQLITE_ROW) {
        return buildFromRow(stmt.getStatement());
    }

    return std::nullopt;
}

std::vector<StudyPreferences> StudyPreferencesRepository::findAllActive() {
    std::vector<StudyPreferences> result;
    if (!database_ || !database_->isOpen()) return result;

    const std::string sql = R"(
        SELECT id, user_id, learning_style, study_environment, study_time_preference,
               courses, topics_of_interest, academic_goals, available_days,
               available_hours_per_week, preferred_group_size, same_university_only,
               same_department_only, same_year_only, is_active, created_at, updated_at
        FROM study_preferences WHERE is_active = 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return result;

    while (stmt.step() == SQLITE_ROW) {
        result.push_back(buildFromRow(stmt.getStatement()));
    }

    return result;
}

std::vector<StudyPreferences> StudyPreferencesRepository::findByLearningStyle(LearningStyle learningStyle) {
    std::vector<StudyPreferences> result;
    if (!database_ || !database_->isOpen()) return result;

    const std::string sql = R"(
        SELECT id, user_id, learning_style, study_environment, study_time_preference,
               courses, topics_of_interest, academic_goals, available_days,
               available_hours_per_week, preferred_group_size, same_university_only,
               same_department_only, same_year_only, is_active, created_at, updated_at
        FROM study_preferences WHERE learning_style = ? AND is_active = 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return result;

    stmt.bindText(1, StudyPreferences::learningStyleToString(learningStyle));

    while (stmt.step() == SQLITE_ROW) {
        result.push_back(buildFromRow(stmt.getStatement()));
    }

    return result;
}

std::vector<StudyPreferences> StudyPreferencesRepository::findByCourses(const std::vector<std::string>& courses) {
    std::vector<StudyPreferences> result;
    if (!database_ || !database_->isOpen() || courses.empty()) return result;

    // Get all active preferences and filter in code for JSON array matching
    auto allPrefs = findAllActive();

    for (const auto& pref : allPrefs) {
        for (const auto& userCourse : pref.courses) {
            for (const auto& searchCourse : courses) {
                if (userCourse == searchCourse) {
                    result.push_back(pref);
                    goto next_pref; // Found a match, move to next preference
                }
            }
        }
        next_pref:;
    }

    return result;
}

bool StudyPreferencesRepository::deactivate(int userId) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "UPDATE study_preferences SET is_active = 0 WHERE user_id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, userId);
    return stmt.step() == SQLITE_DONE;
}

bool StudyPreferencesRepository::deleteByUserId(int userId) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM study_preferences WHERE user_id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, userId);
    return stmt.step() == SQLITE_DONE;
}

StudyPreferences StudyPreferencesRepository::buildFromRow(sqlite3_stmt* stmt) {
    StudyPreferences prefs;

    prefs.id = sqlite3_column_int(stmt, 0);
    prefs.user_id = sqlite3_column_int(stmt, 1);

    const char* learningStyleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    prefs.learning_style = learningStyleStr ?
        StudyPreferences::stringToLearningStyle(learningStyleStr) : LearningStyle::MIXED;

    const char* envStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    prefs.study_environment = envStr ?
        StudyPreferences::stringToStudyEnvironment(envStr) : StudyEnvironment::FLEXIBLE;

    const char* timeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    prefs.study_time_preference = timeStr ?
        StudyPreferences::stringToStudyTimePreference(timeStr) : StudyTimePreference::FLEXIBLE;

    // Parse JSON arrays
    const char* coursesStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    if (coursesStr) {
        try {
            json coursesJson = json::parse(coursesStr);
            if (coursesJson.is_array()) {
                for (const auto& course : coursesJson) {
                    prefs.courses.push_back(course.get<std::string>());
                }
            }
        } catch (...) {}
    }

    const char* topicsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    if (topicsStr) {
        try {
            json topicsJson = json::parse(topicsStr);
            if (topicsJson.is_array()) {
                for (const auto& topic : topicsJson) {
                    prefs.topics_of_interest.push_back(topic.get<std::string>());
                }
            }
        } catch (...) {}
    }

    const char* goalsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    prefs.academic_goals = goalsStr ? goalsStr : "";

    const char* daysStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    if (daysStr) {
        try {
            json daysJson = json::parse(daysStr);
            if (daysJson.is_array()) {
                for (const auto& day : daysJson) {
                    prefs.available_days.push_back(day.get<std::string>());
                }
            }
        } catch (...) {}
    }

    prefs.available_hours_per_week = sqlite3_column_int(stmt, 9);
    prefs.preferred_group_size = sqlite3_column_int(stmt, 10);
    prefs.same_university_only = sqlite3_column_int(stmt, 11) == 1;
    prefs.same_department_only = sqlite3_column_int(stmt, 12) == 1;
    prefs.same_year_only = sqlite3_column_int(stmt, 13) == 1;
    prefs.is_active = sqlite3_column_int(stmt, 14) == 1;
    prefs.created_at = sqlite3_column_int64(stmt, 15);
    prefs.updated_at = sqlite3_column_int64(stmt, 16);

    return prefs;
}

} // namespace repositories
} // namespace sohbet
