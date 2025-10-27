#include "services/permission_service.h"

namespace sohbet {
namespace services {

PermissionService::PermissionService(std::shared_ptr<repositories::RoleRepository> role_repository)
    : role_repository_(role_repository) {}

bool PermissionService::userHasPermission(int user_id, const std::string& permission) {
    if (!role_repository_) return false;
    return role_repository_->userHasPermission(user_id, permission);
}

bool PermissionService::tokenHasPermission(const std::string& token, const std::string& permission) {
    auto payload = security::verify_jwt_token(token);
    if (!payload.has_value()) return false;
    
    return userHasPermission(payload->user_id, permission);
}

std::optional<int> PermissionService::getUserIdFromToken(const std::string& token) {
    auto payload = security::verify_jwt_token(token);
    if (!payload.has_value()) return std::nullopt;
    
    return payload->user_id;
}

std::optional<std::string> PermissionService::getRoleFromToken(const std::string& token) {
    auto payload = security::verify_jwt_token(token);
    if (!payload.has_value()) return std::nullopt;
    
    return payload->role;
}

bool PermissionService::verifyAndCheckPermission(const std::string& token, const std::string& permission) {
    auto payload = security::verify_jwt_token(token);
    if (!payload.has_value()) return false;
    
    return userHasPermission(payload->user_id, permission);
}

} // namespace services
} // namespace sohbet
