#pragma once

#include "models/role.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace sohbet {
namespace repositories {

class RoleRepository {
public:
    explicit RoleRepository(std::shared_ptr<db::Database> database);

    // Find role by ID
    std::optional<Role> findById(int id);
    
    // Find role by name
    std::optional<Role> findByName(const std::string& name);
    
    // Get all roles
    std::vector<Role> findAll();
    
    // Get permissions for a role
    std::vector<std::string> getRolePermissions(int role_id);
    
    // Check if a role has a specific permission
    bool hasPermission(int role_id, const std::string& permission);
    
    // Get user's role
    std::optional<Role> getUserRole(int user_id);
    
    // Check if user has permission (checks user's role)
    bool userHasPermission(int user_id, const std::string& permission);
    
    // Assign role to user
    bool assignRoleToUser(int user_id, int role_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
