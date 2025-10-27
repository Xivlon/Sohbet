#pragma once

#include "repositories/role_repository.h"
#include "security/jwt.h"
#include <memory>
#include <string>
#include <optional>

namespace sohbet {
namespace services {

class PermissionService {
public:
    explicit PermissionService(std::shared_ptr<repositories::RoleRepository> role_repository);

    // Check if a user has a specific permission
    bool userHasPermission(int user_id, const std::string& permission);
    
    // Check if a JWT token has a specific permission
    bool tokenHasPermission(const std::string& token, const std::string& permission);
    
    // Extract user ID from JWT token
    std::optional<int> getUserIdFromToken(const std::string& token);
    
    // Extract role from JWT token
    std::optional<std::string> getRoleFromToken(const std::string& token);
    
    // Verify token and check permission in one call
    bool verifyAndCheckPermission(const std::string& token, const std::string& permission);

private:
    std::shared_ptr<repositories::RoleRepository> role_repository_;
};

} // namespace services
} // namespace sohbet
