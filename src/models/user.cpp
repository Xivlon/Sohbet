#include "models/user.h"
#include <sstream>
#include <regex>

namespace sohbet {

User::User(const std::string& username, const std::string& email)
    : username_(username), email_(email) {}

std::string User::toJson() const {
    std::ostringstream oss;
    oss << "{";
    
    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }
    
    oss << "\"username\":\"" << username_ << "\",";
    oss << "\"email\":\"" << email_ << "\"";
    
    if (university_.has_value()) {
        oss << ",\"university\":\"" << university_.value() << "\"";
    }
    
    if (department_.has_value()) {
        oss << ",\"department\":\"" << department_.value() << "\"";
    }
    
    if (enrollment_year_.has_value()) {
        oss << ",\"enrollment_year\":" << enrollment_year_.value();
    }
    
    if (primary_language_.has_value()) {
        oss << ",\"primary_language\":\"" << primary_language_.value() << "\"";
    }
    
    if (!additional_languages_.empty()) {
        oss << ",\"additional_languages\":[";
        for (size_t i = 0; i < additional_languages_.size(); ++i) {
            oss << "\"" << additional_languages_[i] << "\"";
            if (i < additional_languages_.size() - 1) oss << ",";
        }
        oss << "]";
    }
    
    oss << "}";
    return oss.str();
}

User User::fromJson(const std::string& json) {
    // Basic JSON parsing for required fields
    User user;
    
    // Extract username (required)
    std::regex username_regex("\"username\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch username_match;
    if (std::regex_search(json, username_match, username_regex)) {
        user.setUsername(username_match[1].str());
    }
    
    // Extract email (required)
    std::regex email_regex("\"email\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch email_match;
    if (std::regex_search(json, email_match, email_regex)) {
        user.setEmail(email_match[1].str());
    }
    
    // Extract optional fields
    std::regex university_regex("\"university\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch university_match;
    if (std::regex_search(json, university_match, university_regex)) {
        user.setUniversity(university_match[1].str());
    }
    
    std::regex department_regex("\"department\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch department_match;
    if (std::regex_search(json, department_match, department_regex)) {
        user.setDepartment(department_match[1].str());
    }
    
    std::regex year_regex("\"enrollment_year\"\\s*:\\s*(\\d+)");
    std::smatch year_match;
    if (std::regex_search(json, year_match, year_regex)) {
        user.setEnrollmentYear(std::stoi(year_match[1].str()));
    }
    
    std::regex lang_regex("\"primary_language\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch lang_match;
    if (std::regex_search(json, lang_match, lang_regex)) {
        user.setPrimaryLanguage(lang_match[1].str());
    }
    
    return user;
}

} // namespace sohbet