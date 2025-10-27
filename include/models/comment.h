#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Comment {
public:
    // Constructors
    Comment() = default;
    Comment(int post_id, int author_id, const std::string& content);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getPostId() const { return post_id_; }
    const std::optional<int>& getParentId() const { return parent_id_; }
    int getAuthorId() const { return author_id_; }
    const std::string& getContent() const { return content_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setPostId(int post_id) { post_id_ = post_id; }
    void setParentId(const std::optional<int>& parent_id) { parent_id_ = parent_id; }
    void setAuthorId(int author_id) { author_id_ = author_id; }
    void setContent(const std::string& content) { content_ = content; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }

    // JSON serialization
    std::string toJson() const;
    static Comment fromJson(const std::string& json);

private:
    std::optional<int> id_;
    int post_id_;
    std::optional<int> parent_id_; // NULL for top-level comments
    int author_id_;
    std::string content_;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
};

} // namespace sohbet
