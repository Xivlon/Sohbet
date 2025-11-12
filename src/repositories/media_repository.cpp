#include "repositories/media_repository.h"

namespace sohbet {
namespace repositories {

MediaRepository::MediaRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Media> MediaRepository::create(Media& media) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO user_media (user_id, media_type, storage_key, file_name,
                               file_size, mime_type, url)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, media.getUserId());
    stmt.bindText(2, media.getMediaType());
    stmt.bindText(3, media.getStorageKey());
    stmt.bindText(4, media.getFileName().value_or(""));
    stmt.bindInt(5, media.getFileSize().value_or(0));
    stmt.bindText(6, media.getMimeType().value_or(""));
    stmt.bindText(7, media.getUrl().value_or(""));

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        media.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return media;
    }

    return std::nullopt;
}

std::optional<Media> MediaRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, media_type, storage_key, file_name, 
               file_size, mime_type, url, created_at
        FROM user_media WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);
    
    if (stmt.step() == SQLITE_ROW) {
        return mediaFromStatement(stmt);
    }

    return std::nullopt;
}

std::optional<Media> MediaRepository::findByUserAndType(int user_id, const std::string& media_type) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, user_id, media_type, storage_key, file_name, 
               file_size, mime_type, url, created_at
        FROM user_media 
        WHERE user_id = ? AND media_type = ?
        ORDER BY created_at DESC
        LIMIT 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, user_id);
    stmt.bindText(2, media_type);
    
    if (stmt.step() == SQLITE_ROW) {
        return mediaFromStatement(stmt);
    }

    return std::nullopt;
}

std::vector<Media> MediaRepository::findByUser(int user_id) {
    std::vector<Media> media_list;
    if (!database_ || !database_->isOpen()) return media_list;

    const std::string sql = R"(
        SELECT id, user_id, media_type, storage_key, file_name, 
               file_size, mime_type, url, created_at
        FROM user_media 
        WHERE user_id = ?
        ORDER BY created_at DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return media_list;

    stmt.bindInt(1, user_id);

    while (stmt.step() == SQLITE_ROW) {
        media_list.push_back(mediaFromStatement(stmt));
    }

    return media_list;
}

bool MediaRepository::updateUrl(int id, const std::string& url) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE user_media SET url = ? WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, url);
    stmt.bindInt(2, id);

    return stmt.step() == SQLITE_DONE;
}

bool MediaRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM user_media WHERE id = ?";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool MediaRepository::deleteByUserAndType(int user_id, const std::string& media_type) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM user_media WHERE user_id = ? AND media_type = ?";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user_id);
    stmt.bindText(2, media_type);
    return stmt.step() == SQLITE_DONE;
}

Media MediaRepository::mediaFromStatement(db::Statement& stmt) {
    Media media;
    media.setId(stmt.getInt(0));
    media.setUserId(stmt.getInt(1));
    media.setMediaType(stmt.getText(2));
    media.setStorageKey(stmt.getText(3));
    
    std::string file_name = stmt.getText(4);
    if (!file_name.empty()) media.setFileName(file_name);
    
    int file_size = stmt.getInt(5);
    if (file_size > 0) media.setFileSize(file_size);
    
    std::string mime_type = stmt.getText(6);
    if (!mime_type.empty()) media.setMimeType(mime_type);
    
    std::string url = stmt.getText(7);
    if (!url.empty()) media.setUrl(url);
    
    std::string created = stmt.getText(8);
    if (!created.empty()) media.setCreatedAt(created);
    
    return media;
}

} // namespace repositories
} // namespace sohbet
