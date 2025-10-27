#pragma once

#include <string>
#include <optional>
#include <vector>

namespace sohbet {

class Post {
public:
    // Constructors
    Post() = default;
    Post(int author_id, const std::string& content);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getAuthorId() const { return author_id_; }
    const std::string& getAuthorType() const { return author_type_; }
    const std::string& getContent() const { return content_; }
    const std::optional<std::string>& getMediaUrls() const { return media_urls_; }
    const std::string& getVisibility() const { return visibility_; }
    const std::optional<int>& getGroupId() const { return group_id_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setAuthorId(int author_id) { author_id_ = author_id; }
    void setAuthorType(const std::string& author_type) { author_type_ = author_type; }
    void setContent(const std::string& content) { content_ = content; }
    void setMediaUrls(const std::optional<std::string>& media_urls) { media_urls_ = media_urls; }
    void setVisibility(const std::string& visibility) { visibility_ = visibility; }
    void setGroupId(const std::optional<int>& group_id) { group_id_ = group_id; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::optional<std::string>& updated_at) { updated_at_ = updated_at; }

    // JSON serialization
    std::string toJson() const;
    static Post fromJson(const std::string& json);

    // Visibility constants
    static constexpr const char* VISIBILITY_PUBLIC = "public";
    static constexpr const char* VISIBILITY_FRIENDS = "friends";
    static constexpr const char* VISIBILITY_PRIVATE = "private";
    static constexpr const char* VISIBILITY_GROUP = "group";

    // Author type constants
    static constexpr const char* AUTHOR_TYPE_USER = "user";
    static constexpr const char* AUTHOR_TYPE_ORGANIZATION = "organization";

private:
    std::optional<int> id_;
    int author_id_;
    std::string author_type_ = AUTHOR_TYPE_USER;
    std::string content_;
    std::optional<std::string> media_urls_; // JSON array as string
    std::string visibility_ = VISIBILITY_FRIENDS;
    std::optional<int> group_id_;
    std::optional<std::string> created_at_;
    std::optional<std::string> updated_at_;
};

} // namespace sohbet
