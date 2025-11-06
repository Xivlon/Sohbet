#pragma once

#include "models/announcement.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

class AnnouncementRepository {
public:
    explicit AnnouncementRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Announcement> create(Announcement& announcement);
    std::optional<Announcement> findById(int id);
    std::vector<Announcement> findByGroupId(int group_id, bool pinned_only = false, int limit = 50, int offset = 0);
    bool update(const Announcement& announcement);
    bool deleteById(int id);

    // Pin/unpin operations
    bool pin(int id);
    bool unpin(int id);
    std::vector<Announcement> findPinnedByGroupId(int group_id);

    // Permission checks
    bool canUserManage(int announcement_id, int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
