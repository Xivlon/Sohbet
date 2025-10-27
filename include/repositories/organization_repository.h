#pragma once

#include "models/organization.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace sohbet {
namespace repositories {

class OrganizationRepository {
public:
    explicit OrganizationRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Organization> create(Organization& org);
    std::optional<Organization> findById(int id);
    std::optional<Organization> findByName(const std::string& name);
    std::vector<Organization> findAll(int limit = 50, int offset = 0);
    std::vector<Organization> findByType(const std::string& type);
    bool update(const Organization& org);
    bool deleteById(int id);
    
    // Account management
    bool addAccount(int org_id, int user_id, const std::string& role = "editor");
    bool removeAccount(int org_id, int user_id);
    bool updateAccountRole(int org_id, int user_id, const std::string& role);
    bool isAccountManager(int org_id, int user_id);
    std::string getAccountRole(int org_id, int user_id);
    
    // Permission checks
    bool canUserManage(int org_id, int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
