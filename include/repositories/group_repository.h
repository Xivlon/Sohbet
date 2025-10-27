#pragma once

#include "models/group.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace sohbet {
namespace repositories {

class GroupRepository {
public:
    explicit GroupRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Group> create(Group& group);
    std::optional<Group> findById(int id);
    std::vector<Group> findAll(int limit = 50, int offset = 0);
    std::vector<Group> findByCreator(int creator_id);
    std::vector<Group> findByMember(int user_id);
    bool update(const Group& group);
    bool deleteById(int id);
    
    // Member management
    bool addMember(int group_id, int user_id, const std::string& role = "member");
    bool removeMember(int group_id, int user_id);
    bool updateMemberRole(int group_id, int user_id, const std::string& role);
    bool isMember(int group_id, int user_id);
    std::string getMemberRole(int group_id, int user_id);
    int getMemberCount(int group_id);
    
    // Permission checks
    bool canUserManage(int group_id, int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
