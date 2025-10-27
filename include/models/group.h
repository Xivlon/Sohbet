#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Group {
public:
    // Constructors
    Group() = default;
    Group(const std::string& name, int creator_id);

    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::optional<std::string>& getDescription() const { return description_; }
    int getCreatorId() const { return creator_id_; }
    const std::string& getPrivacy() const { return privacy_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::optional<std::string>& description) { description_ = description; }
    void setCreatorId(int creator_id) { creator_id_ = creator_id; }
    void setPrivacy(const std::string& privacy) { privacy_ = privacy; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }

    // JSON serialization
    std::string toJson() const;
    static Group fromJson(const std::string& json);

    // Privacy constants
    static constexpr const char* PRIVACY_PUBLIC = "public";
    static constexpr const char* PRIVACY_PRIVATE = "private";
    static constexpr const char* PRIVACY_INVITE_ONLY = "invite_only";

private:
    std::optional<int> id_;
    std::string name_;
    std::optional<std::string> description_;
    int creator_id_;
    std::string privacy_ = PRIVACY_PRIVATE;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
};

} // namespace sohbet
