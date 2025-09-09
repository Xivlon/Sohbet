#include "models/user.h"
#include <regex>
#include <sstream>
#include <algorithm>

namespace sohbet {

User::User(const std::string& username, const std::string& email, const std::string& password_hash)
    : username_(username), email_(email), password_hash_(password_hash) {
}

std::string User::toJson() const {
    std::ostringstream oss;
    oss << "{";
    
    if (id_.has_value()) {
        oss << "\"id\":" << id_.value() << ",";
    }
    
    oss << "\"username\":\"" << username_ << "\",";
    oss << "\"email\":\"" << email_ << "\"";
    
    if (!university_.empty()) {
        oss << ",\"university\":\"" << university_ << "\"";
    }
    
    if (!department_.empty()) {
        oss << ",\"department\":\"" << department_ << "\"";
    }
    
    if (enrollment_year_.has_value()) {
        oss << ",\"enrollment_year\":" << enrollment_year_.value();
    }
    
    if (!primary_language_.empty()) {
        oss << ",\"primary_language\":\"" << primary_language_ << "\"";
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

void User::fromJson(const std::string& json) {
    // Simple JSON parsing - in production, use a proper JSON library
    // This is a basic implementation for the prototype
    
    // Extract username
    std::regex username_regex("\"username\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    if (std::regex_search(json, match, username_regex)) {
        username_ = match[1].str();
    }
    
    // Extract email
    std::regex email_regex("\"email\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, email_regex)) {
        email_ = match[1].str();
    }
    
    // Extract university
    std::regex university_regex("\"university\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, university_regex)) {
        university_ = match[1].str();
    }
    
    // Extract department
    std::regex department_regex("\"department\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, department_regex)) {
        department_ = match[1].str();
    }
    
    // Extract enrollment_year
    std::regex year_regex("\"enrollment_year\"\\s*:\\s*(\\d+)");
    if (std::regex_search(json, match, year_regex)) {
        enrollment_year_ = std::stoi(match[1].str());
    }
    
    // Extract primary_language
    std::regex lang_regex("\"primary_language\"\\s*:\\s*\"([^\"]*)\"");
    if (std::regex_search(json, match, lang_regex)) {
        primary_language_ = match[1].str();
    }
}

bool User::isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 32) {
        return false;
    }
    
    std::regex pattern("^[A-Za-z0-9_]{3,32}$");
    return std::regex_match(username, pattern);
}

bool User::isValidEmail(const std::string& email) {
    return email.find('@') != std::string::npos && !email.empty();
}

bool User::isValidPassword(const std::string& password) {
    return password.length() >= 8;
}

} // namespace sohbet