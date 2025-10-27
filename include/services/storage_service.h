#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace sohbet {
namespace services {

/**
 * File metadata returned from storage operations
 */
struct FileMetadata {
    std::string storage_key;
    std::string file_name;
    std::string mime_type;
    size_t file_size;
    std::string url;
};

/**
 * Storage service for handling file uploads and retrieval
 * Currently implements local filesystem storage
 */
class StorageService {
public:
    /**
     * Constructor
     * @param storage_path Base path for storing files (default: "uploads/")
     */
    explicit StorageService(const std::string& storage_path = "uploads/");
    
    /**
     * Store a file
     * @param file_data Binary file data
     * @param file_name Original filename
     * @param mime_type MIME type of the file
     * @param user_id User ID uploading the file
     * @param media_type Type of media (avatar, banner, post_image, etc.)
     * @return File metadata if successful, std::nullopt otherwise
     */
    std::optional<FileMetadata> storeFile(
        const std::vector<uint8_t>& file_data,
        const std::string& file_name,
        const std::string& mime_type,
        int user_id,
        const std::string& media_type
    );
    
    /**
     * Retrieve a file
     * @param storage_key Storage key of the file
     * @return File data if found, std::nullopt otherwise
     */
    std::optional<std::vector<uint8_t>> retrieveFile(const std::string& storage_key);
    
    /**
     * Delete a file
     * @param storage_key Storage key of the file
     * @return true if deleted successfully, false otherwise
     */
    bool deleteFile(const std::string& storage_key);
    
    /**
     * Check if file exists
     * @param storage_key Storage key of the file
     * @return true if file exists, false otherwise
     */
    bool fileExists(const std::string& storage_key);
    
    /**
     * Validate file type
     * @param mime_type MIME type to validate
     * @param allowed_types List of allowed MIME types (empty = allow all)
     * @return true if valid, false otherwise
     */
    static bool validateFileType(
        const std::string& mime_type,
        const std::vector<std::string>& allowed_types = {}
    );
    
    /**
     * Validate file size
     * @param file_size Size in bytes
     * @param max_size Maximum allowed size in bytes
     * @return true if valid, false otherwise
     */
    static bool validateFileSize(size_t file_size, size_t max_size);
    
    /**
     * Generate storage key
     * @param user_id User ID
     * @param media_type Media type
     * @param file_name Original filename
     * @return Unique storage key
     */
    static std::string generateStorageKey(
        int user_id,
        const std::string& media_type,
        const std::string& file_name
    );
    
    /**
     * Get the full filesystem path for a storage key
     * @param storage_key Storage key
     * @return Full path to file
     */
    std::string getFilePath(const std::string& storage_key) const;

private:
    std::string storage_path_;
    
    /**
     * Ensure storage directory exists
     * @return true if successful, false otherwise
     */
    bool ensureStorageDirectory();
    
    /**
     * Extract file extension from filename
     * @param file_name Filename
     * @return File extension (with dot) or empty string
     */
    static std::string getFileExtension(const std::string& file_name);
};

} // namespace services
} // namespace sohbet
