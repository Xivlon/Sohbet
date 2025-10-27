#include "models/user.h"
#include <regex>
#include <sstream>
#include <algorithm>

namespace sohbet {

// Constructor
User::User(const std::string& username, const std::string& email)
    : username_(username), email_(email) {}

// Serialize to JSON (excludes password hash)
std::string User::toJson() const {
    std::ostringstream oss;
    oss << "{";

    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }

    oss << "\"username\":\"" << username_ << "\",";
    oss << "\"email\":\"" << email_ << "\"";

    if (university_.has_value() && !university_.value().empty()) {
        oss << ",\"university\":\"" << university_.value() << "\"";
    }

    if (department_.has_value() && !department_.value().empty()) {
        oss << ",\"department\":\"" << department_.value() << "\"";
    }

    if (enrollment_year_.has_value()) {
        oss << ",\"enrollment_year\":" << enrollment_year_.value();
    }

    if (primary_language_.has_value() && !primary_language_.value().empty()) {
        oss << ",\"primary_language\":\"" << primary_language_.value() << "\"";
    }

    if (!additional_languages_.empty()) {
        oss << ",\"additional_languages\":[";
        for (size_t i = 0; i < additional_languages_.size(); ++i) {
            oss << "\"" << additional_languages_[i] << "\"";
            if (i < additional_languages_.size() - 1) {
                oss << ",";
            }
        }
        oss << "]";
    }

    oss << "}";
    return oss.str();
}

// Deserialize from JSON (basic prototype parser)
User User::fromJson(const std::string& json) {
    User user;
    std::smatch match;

    // Username
    std::regex username_regex("\"username\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, username_regex)) {
        user.username_ = match[1].str();
    }

    // Email
    std::regex email_regex("\"email\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, email_regex)) {
        user.email_ = match[1].str();
    }

    // University
    std::regex university_regex("\"university\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, university_regex)) {
        user.university_ = match[1].str();
    }

    // Department
    std::regex department_regex("\"department\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, department_regex)) {
        user.department_ = match[1].str();
    }

    // Enrollment year
    std::regex year_regex("\"enrollment_year\"\\s*:\\s*(\\d+)");
    if (std::regex_search(json, match, year_regex)) {
        user.enrollment_year_ = std::stoi(match[1].str());
    }

    // Primary language
    std::regex lang_regex("\"primary_language\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, lang_regex)) {
        user.primary_language_ = match[1].str();
    }

    return user;
}

// Validation utilities
bool User::isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 32) return false;
    std::regex pattern("^[A-Za-z0-9_]{3,32}$");
    return std::regex_match(username, pattern);
}

bool User::isValidEmail(const std::string& email) {
    return !email.empty() && email.find('@') != std::string::npos;
}

bool User::isValidPassword(const std::string& password) {
    return password.length() >= 8;
}

} // namespace sohbet