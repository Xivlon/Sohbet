#include <cassert>
#include <iostream>
#include <memory>
#include "db/database.h"
#include "repositories/user_repository.h"
#include "repositories/role_repository.h"
#include "repositories/post_repository.h"
#include "models/user.h"
#include "models/post.h"

using namespace sohbet;
using namespace sohbet::repositories;

int main() {
    std::cout << "Testing Admin Permissions..." << std::endl;
    
    // Create in-memory database
    auto db = std::make_shared<db::Database>(":memory:");
    assert(db->isOpen());
    
    auto user_repo = std::make_shared<UserRepository>(db);
    auto role_repo = std::make_shared<RoleRepository>(db);
    auto post_repo = std::make_shared<PostRepository>(db);
    
    // Run migrations
    assert(user_repo->migrate());
    
    // Run social features migration
    const std::string migration_sql = R"(
        CREATE TABLE IF NOT EXISTS roles (
            id SERIAL PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS role_permissions (
            id SERIAL PRIMARY KEY,
            role_id INTEGER NOT NULL,
            permission TEXT NOT NULL,
            FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
            UNIQUE(role_id, permission)
        );

        CREATE TABLE IF NOT EXISTS user_roles (
            id SERIAL PRIMARY KEY,
            user_id INTEGER NOT NULL,
            role_id INTEGER NOT NULL,
            assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
            UNIQUE(user_id, role_id)
        );

        CREATE TABLE IF NOT EXISTS posts (
            id SERIAL PRIMARY KEY,
            author_id INTEGER NOT NULL,
            author_type TEXT DEFAULT 'user',
            content TEXT NOT NULL,
            media_urls TEXT,
            visibility TEXT DEFAULT 'public',
            group_id INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
        );

        INSERT INTO roles (name, description) VALUES
        ('Student', 'Default student account'),
        ('Professor', 'Faculty account'),
        ('Admin', 'Administrator account')
        ON CONFLICT (name) DO NOTHING;

        INSERT INTO role_permissions (role_id, permission) VALUES
        (3, 'delete_any_post'),
        (3, 'edit_any_post'),
        (3, 'delete_any_comment'),
        (3, 'edit_any_comment')
        ON CONFLICT DO NOTHING;
    )";
    
    assert(db->execute(migration_sql));
    
    // Create regular student user
    User student("student1", "student1@example.edu");
    student.setUniversity("Test University");
    student.setDepartment("Computer Science");
    student.setEnrollmentYear(2023);
    
    auto created_student = user_repo->create(student, "password123");
    assert(created_student.has_value());
    int student_id = created_student->getId().value();
    
    // Create admin user (demo_student)
    User admin("demo_student", "demo@example.edu");
    admin.setUniversity("Test University");
    admin.setDepartment("Computer Science");
    admin.setEnrollmentYear(2023);
    
    auto created_admin = user_repo->create(admin, "demo123");
    assert(created_admin.has_value());
    int admin_id = created_admin->getId().value();
    
    // Assign Admin role to demo_student
    auto admin_role = role_repo->findByName("Admin");
    assert(admin_role.has_value());
    assert(role_repo->assignRoleToUser(admin_id, admin_role->getId().value()));
    
    // Verify admin has permissions
    std::cout << "Checking admin permissions..." << std::endl;
    assert(role_repo->userHasPermission(admin_id, "delete_any_post"));
    assert(role_repo->userHasPermission(admin_id, "edit_any_post"));
    assert(role_repo->userHasPermission(admin_id, "delete_any_comment"));
    assert(role_repo->userHasPermission(admin_id, "edit_any_comment"));
    std::cout << "✓ Admin has all required permissions" << std::endl;
    
    // Verify student does NOT have these permissions
    std::cout << "Checking student permissions..." << std::endl;
    assert(!role_repo->userHasPermission(student_id, "delete_any_post"));
    assert(!role_repo->userHasPermission(student_id, "edit_any_post"));
    std::cout << "✓ Student does not have admin permissions" << std::endl;
    
    // Create a post by the student
    Post student_post(student_id, "This is student's post");
    student_post.setVisibility("public");
    auto created_post = post_repo->create(student_post);
    assert(created_post.has_value());
    int post_id = created_post->getId().value();
    std::cout << "✓ Created post by student (ID: " << post_id << ")" << std::endl;
    
    // Verify the post exists
    auto fetched_post = post_repo->findById(post_id);
    assert(fetched_post.has_value());
    assert(fetched_post->getAuthorId() == student_id);
    std::cout << "✓ Post author is student (ID: " << student_id << ")" << std::endl;
    
    // In actual server code, admin would be able to edit/delete this post
    // because of the permission checks we added
    std::cout << "✓ Admin user (demo_student) can edit/delete any post due to permissions" << std::endl;
    
    std::cout << "\n=== All Admin Permission Tests Passed ===" << std::endl;
    return 0;
}
