#include "services/storage_service.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <random>

namespace sohbet {
namespace services {

StorageService::StorageService(const std::string& storage_path)
    : storage_path_(storage_path) {
    // Ensure trailing slash
    if (!storage_path_.empty() && storage_path_.back() != '/') {
        storage_path_ += '/';
    }
    ensureStorageDirectory();
}

bool StorageService::ensureStorageDirectory() {
    struct stat st;
    if (stat(storage_path_.c_str(), &st) != 0) {
        // Directory doesn't exist, create it
        if (mkdir(storage_path_.c_str(), 0755) != 0) {
            return false;
        }
    }
    return true;
}

std::optional<FileMetadata> StorageService::storeFile(
    const std::vector<uint8_t>& file_data,
    const std::string& file_name,
    const std::string& mime_type,
    int user_id,
    const std::string& media_type
) {
    if (file_data.empty() || file_name.empty()) {
        return std::nullopt;
    }
    
    if (!ensureStorageDirectory()) {
        return std::nullopt;
    }
    
    // Generate unique storage key
    std::string storage_key = generateStorageKey(user_id, media_type, file_name);
    std::string file_path = getFilePath(storage_key);
    
    // Write file to disk
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    file.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
    file.close();
    
    if (!file.good()) {
        return std::nullopt;
    }
    
    // Create metadata
    FileMetadata metadata;
    metadata.storage_key = storage_key;
    metadata.file_name = file_name;
    metadata.mime_type = mime_type;
    metadata.file_size = file_data.size();
    metadata.url = "/api/media/file/" + storage_key;
    
    return metadata;
}

std::optional<std::vector<uint8_t>> StorageService::retrieveFile(const std::string& storage_key) {
    std::string file_path = getFilePath(storage_key);
    
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return std::nullopt;
    }
    
    return buffer;
}

bool StorageService::deleteFile(const std::string& storage_key) {
    std::string file_path = getFilePath(storage_key);
    return unlink(file_path.c_str()) == 0;
}

bool StorageService::fileExists(const std::string& storage_key) {
    std::string file_path = getFilePath(storage_key);
    struct stat st;
    return stat(file_path.c_str(), &st) == 0;
}

bool StorageService::validateFileType(
    const std::string& mime_type,
    const std::vector<std::string>& allowed_types
) {
    if (allowed_types.empty()) {
        return true; // No restrictions
    }
    
    return std::find(allowed_types.begin(), allowed_types.end(), mime_type) != allowed_types.end();
}

bool StorageService::validateFileSize(size_t file_size, size_t max_size) {
    return file_size <= max_size;
}

std::string StorageService::generateStorageKey(
    int user_id,
    const std::string& media_type,
    const std::string& file_name
) {
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    
    // Generate random component
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    int random_part = dis(gen);
    
    // Extract file extension
    std::string extension = getFileExtension(file_name);
    
    // Format: user_{user_id}_{media_type}_{timestamp}_{random}{extension}
    std::ostringstream oss;
    oss << "user_" << user_id << "_" << media_type << "_" 
        << timestamp << "_" << random_part << extension;
    
    return oss.str();
}

std::string StorageService::getFileExtension(const std::string& file_name) {
    size_t dot_pos = file_name.find_last_of('.');
    if (dot_pos != std::string::npos && dot_pos < file_name.length() - 1) {
        return file_name.substr(dot_pos);
    }
    return "";
}

std::string StorageService::getFilePath(const std::string& storage_key) const {
    return storage_path_ + storage_key;
}

} // namespace services
} // namespace sohbet
