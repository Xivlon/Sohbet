#pragma once

#include <string>
#include <optional>

namespace sohbet {

class Media {
public:
    // Constructors
    Media() = default;
    Media(int user_id, const std::string& media_type, const std::string& storage_key);

    // Getters
    std::optional<int> getId() const { return id_; }
    int getUserId() const { return user_id_; }
    const std::string& getMediaType() const { return media_type_; }
    const std::string& getStorageKey() const { return storage_key_; }
    const std::optional<std::string>& getFileName() const { return file_name_; }
    const std::optional<int>& getFileSize() const { return file_size_; }
    const std::optional<std::string>& getMimeType() const { return mime_type_; }
    const std::optional<std::string>& getUrl() const { return url_; }
    const std::optional<std::string>& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setMediaType(const std::string& media_type) { media_type_ = media_type; }
    void setStorageKey(const std::string& storage_key) { storage_key_ = storage_key; }
    void setFileName(const std::optional<std::string>& file_name) { file_name_ = file_name; }
    void setFileSize(const std::optional<int>& file_size) { file_size_ = file_size; }
    void setMimeType(const std::optional<std::string>& mime_type) { mime_type_ = mime_type; }
    void setUrl(const std::optional<std::string>& url) { url_ = url; }
    void setCreatedAt(const std::optional<std::string>& created) { created_at_ = created; }

    // JSON serialization
    std::string toJson() const;

private:
    std::optional<int> id_;
    int user_id_;
    std::string media_type_;
    std::string storage_key_;
    std::optional<std::string> file_name_;
    std::optional<int> file_size_;
    std::optional<std::string> mime_type_;
    std::optional<std::string> url_;
    std::optional<std::string> created_at_;
};

} // namespace sohbet
