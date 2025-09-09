#pragma once

#include <string>
#include <vector>
#include <optional>

namespace sohbet {

class User {
public:
    // Constructors
    User() = default;
    User(const std::string& username, const std::string& email, const std::string& password_hash);
    
    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getUniversity() const { return university_; }
    const std::string& getDepartment() const { return department_; }
    std::optional<int> getEnrollmentYear() const { return enrollment_year_; }
    const std::string& getPrimaryLanguage() const { return primary_language_; }
    const std::vector<std::string>& getAdditionalLanguages() const { return additional_languages_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setUniversity(const std::string& university) { university_ = university; }
    void setDepartment(const std::string& department) { department_ = department; }
    void setEnrollmentYear(int year) { enrollment_year_ = year; }
    void setPrimaryLanguage(const std::string& language) { primary_language_ = language; }
    void setAdditionalLanguages(const std::vector<std::string>& languages) { additional_languages_ = languages; }
    
    // Password hash access (internal only, never serialized)
    const std::string& getPasswordHash() const { return password_hash_; }
    void setPasswordHash(const std::string& hash) { password_hash_ = hash; }
    
    // JSON serialization (excludes password hash)
    std::string toJson() const;
    void fromJson(const std::string& json);
    
    // Validation
    static bool isValidUsername(const std::string& username);
    static bool isValidEmail(const std::string& email);
    static bool isValidPassword(const std::string& password);

private:
    std::optional<int> id_;
    std::string username_;
    std::string email_;
    std::string password_hash_;  // Internal only, never serialized
    std::string university_;
    std::string department_;
    std::optional<int> enrollment_year_;
    std::string primary_language_;
    std::vector<std::string> additional_languages_;
};

} // namespace sohbet