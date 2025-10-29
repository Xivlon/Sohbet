#pragma once

#include <string>
#include <vector>
#include <optional>

namespace sohbet {

class User {
public:
    // Constructors
    User() = default;
    User(const std::string& username, const std::string& email);

    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::optional<std::string>& getName() const { return name_; }
    const std::optional<std::string>& getPosition() const { return position_; }
    const std::optional<std::string>& getPhoneNumber() const { return phone_number_; }
    const std::optional<std::string>& getUniversity() const { return university_; }
    const std::optional<std::string>& getDepartment() const { return department_; }
    const std::optional<int>& getEnrollmentYear() const { return enrollment_year_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<int>& getWarnings() const { return warnings_; }
    const std::optional<std::string>& getPrimaryLanguage() const { return primary_language_; }
    const std::vector<std::string>& getAdditionalLanguages() const { return additional_languages_; }
    const std::optional<std::string>& getRole() const { return role_; }
    const std::optional<std::string>& getAvatarUrl() const { return avatar_url_; }
    const std::optional<std::string>& getBannerUrl() const { return banner_url_; }
    bool getEmailVerified() const { return email_verified_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setName(const std::optional<std::string>& name) { name_ = name; }
    void setPosition(const std::optional<std::string>& position) { position_ = position; }
    void setPhoneNumber(const std::optional<std::string>& phone) { phone_number_ = phone; }
    void setUniversity(const std::optional<std::string>& university) { university_ = university; }
    void setDepartment(const std::optional<std::string>& department) { department_ = department; }
    void setEnrollmentYear(const std::optional<int>& year) { enrollment_year_ = year; }
    void setCreatedAt(const std::optional<std::string>& created) { created_at_ = created; }
    void setWarnings(const std::optional<int>& warnings) { warnings_ = warnings; }
    void setPrimaryLanguage(const std::optional<std::string>& language) { primary_language_ = language; }
    void setAdditionalLanguages(const std::vector<std::string>& languages) { additional_languages_ = languages; }
    void setRole(const std::optional<std::string>& role) { role_ = role; }
    void setAvatarUrl(const std::optional<std::string>& avatar_url) { avatar_url_ = avatar_url; }
    void setBannerUrl(const std::optional<std::string>& banner_url) { banner_url_ = banner_url; }
    void setEmailVerified(bool verified) { email_verified_ = verified; }

    // Password management (internal only, not serialized)
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    const std::string& getPasswordHash() const { return password_hash_; }

    // JSON serialization (excludes password hash)
    std::string toJson() const;
    static User fromJson(const std::string& json);

    // Validation helpers
    static bool isValidUsername(const std::string& username);
    static bool isValidEmail(const std::string& email);
    static bool isValidPassword(const std::string& password);

private:
    std::optional<int> id_;
    std::string username_;
    std::string email_;
    std::optional<std::string> name_;
    std::optional<std::string> position_;
    std::optional<std::string> phone_number_;
    std::optional<std::string> university_;
    std::optional<std::string> department_;
    std::optional<int> enrollment_year_;
    std::optional<std::string> created_at_;
    std::optional<int> warnings_;
    std::optional<std::string> primary_language_;
    std::vector<std::string> additional_languages_;
    std::optional<std::string> role_;
    std::optional<std::string> avatar_url_;
    std::optional<std::string> banner_url_;
    bool email_verified_ = false;
    std::string password_hash_; // Not serialized in JSON
};

} // namespace sohbet