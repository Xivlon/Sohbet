#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Role {
public:
    // Constructors
    Role() = default;
    Role(const std::string& name, const std::string& description = "");

    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setCreatedAt(const std::optional<std::string>& created) { created_at_ = created; }

    // JSON serialization
    std::string toJson() const;
    static Role fromJson(const std::string& json);

private:
    std::optional<int> id_;
    std::string name_;
    std::string description_;
    std::optional<std::string> created_at_;
};

} // namespace sohbet
