#pragma once

#include <string>
#include <optional>
#include <vector>

namespace sohbet {

class Hashtag {
public:
    // Constructors
    Hashtag() = default;
    Hashtag(const std::string& tag);

    // Getters
    std::optional<int> getId() const { return id_; }
    const std::string& getTag() const { return tag_; }
    int getUsageCount() const { return usage_count_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }
    const std::optional<std::string>& getLastUsedAt() const { return last_used_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setTag(const std::string& tag) { tag_ = tag; }
    void setUsageCount(int count) { usage_count_ = count; }
    void setCreatedAt(const std::optional<std::string>& created_at) { created_at_ = created_at; }
    void setLastUsedAt(const std::optional<std::string>& last_used_at) { last_used_at_ = last_used_at; }

    // Helpers
    void incrementUsage() { usage_count_++; }

    // JSON serialization
    std::string toJson() const;
    static Hashtag fromJson(const std::string& json);

private:
    std::optional<int> id_;
    std::string tag_;
    int usage_count_ = 0;
    std::optional<std::string> created_at_;
    std::optional<std::string> last_used_at_;
};

} // namespace sohbet
