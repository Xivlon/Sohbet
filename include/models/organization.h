#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Organization {
public:
    // Constructors
    Organization() = default;
    Organization(const std::string& name, const std::string& type);

    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getType() const { return type_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    const std::optional<std::string>& getEmail() const { return email_; }
    const std::optional<std::string>& getWebsite() const { return website_; }
    const std::optional<std::string>& getLogoUrl() const { return logo_url_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setType(const std::string& type) { type_ = type; }
    void setDescription(const std::optional<std::string>& description) { description_ = description; }
    void setEmail(const std::optional<std::string>& email) { email_ = email; }
    void setWebsite(const std::optional<std::string>& website) { website_ = website; }
    void setLogoUrl(const std::optional<std::string>& logo_url) { logo_url_ = logo_url; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }

    // JSON serialization
    std::string toJson() const;
    static Organization fromJson(const std::string& json);

    // Type constants
    static constexpr const char* TYPE_CLUB = "club";
    static constexpr const char* TYPE_DEPARTMENT = "department";
    static constexpr const char* TYPE_SOCIETY = "society";

private:
    std::optional<int> id_;
    std::string name_;
    std::string type_;
    std::optional<std::string> description_;
    std::optional<std::string> email_;
    std::optional<std::string> website_;
    std::optional<std::string> logo_url_;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
};

} // namespace sohbet
