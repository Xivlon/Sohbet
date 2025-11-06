#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Announcement {
public:
    // Constructors
    Announcement() = default;
    Announcement(int group_id, int author_id, const std::string& title, const std::string& content);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getGroupId() const { return group_id_; }
    int getAuthorId() const { return author_id_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getContent() const { return content_; }
    bool isPinned() const { return is_pinned_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }
    const std::optional<std::string>& getAuthorUsername() const { return author_username_; }
    const std::optional<std::string>& getAuthorName() const { return author_name_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setGroupId(int group_id) { group_id_ = group_id; }
    void setAuthorId(int author_id) { author_id_ = author_id; }
    void setTitle(const std::string& title) { title_ = title; }
    void setContent(const std::string& content) { content_ = content; }
    void setPinned(bool pinned) { is_pinned_ = pinned; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }
    void setAuthorUsername(const std::optional<std::string>& username) { author_username_ = username; }
    void setAuthorName(const std::optional<std::string>& name) { author_name_ = name; }

    // JSON serialization
    std::string toJson() const;
    static Announcement fromJson(const std::string& json);

private:
    std::optional<int> id_;
    int group_id_;
    int author_id_;
    std::string title_;
    std::string content_;
    bool is_pinned_ = false;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
    // Author information (populated from JOIN with users table)
    std::optional<std::string> author_username_;
    std::optional<std::string> author_name_;
};

} // namespace sohbet
