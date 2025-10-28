# User Helpers and Migration System Implementation

## Overview

This implementation adds high-level C++ user helper functions, a database migration system, and a startup initialization program to the Sohbet project as requested.

## Components Implemented

### 1. User Helper Functions

**Location:** `include/helpers/user_helpers.h`, `src/helpers/user_helpers.cpp`

The `UserHelpers` class provides convenient wrappers around `UserRepository` with built-in validation and error handling:

- **`createUser()`**: Creates a new user with automatic password hashing and validation
  - Validates username (3-32 alphanumeric characters or underscores)
  - Validates email format
  - Validates password strength (minimum 8 characters)
  - Checks for duplicate usernames and emails
  - Automatically hashes passwords using bcrypt
  
- **`getUserByUsername()`**: Retrieves a user by username
  
- **`getUserById()`**: Retrieves a user by ID

- **`updateUser()`**: Updates user profile information (name, position, phone, university, department, enrollment year, language)

- **`authenticateUser()`**: Authenticates a user with username and password
  - Retrieves user from database
  - Verifies password using bcrypt
  - Returns user object on success

All functions return `std::optional` and set descriptive error messages accessible via `getLastError()`.

### 2. Migration System

**Location:** `include/db/migration_runner.h`, `src/db/migration_runner.cpp`

The `MigrationRunner` class manages database schema versioning and migrations:

**Features:**
- Creates and manages `schema_migrations` table to track applied migrations
- Loads SQL migration files from a directory (format: `001_name.sql`)
- Applies pending migrations in order
- Tracks current schema version
- Prevents duplicate migration application

**Key Methods:**
- `initialize()`: Creates migration tracking table
- `loadMigrationsFromDirectory()`: Loads migration files
- `applyPendingMigrations()`: Applies all pending migrations
- `getCurrentVersion()`: Returns current schema version
- `isMigrationApplied()`: Checks if a migration has been applied

### 3. Database Initializer

**Location:** `include/init/database_initializer.h`, `src/init/database_initializer.cpp`

The `DatabaseInitializer` class provides startup initialization:

**Features:**
- Opens database connection
- Initializes migration system
- Loads migration files from directory
- Automatically applies pending migrations
- Ensures schema exists on first run

**Usage:**
```cpp
sohbet::init::DatabaseInitializer initializer("myapp.db", "migrations");
if (!initializer.initialize()) {
    std::cerr << "Init failed: " << initializer.getLastError() << std::endl;
    return 1;
}
auto db = initializer.getDatabase();
```

### 4. Migration Files

**Location:** `migrations/`

- `000_create_users.sql`: Creates the core users table
- `001_social_features.sql`: Existing social features migration

Migration files are numbered sequentially and named descriptively (e.g., `002_add_feature.sql`).

## Usage Examples

### Creating a User (e.g., from a registration form)

```cpp
auto db = std::make_shared<sohbet::db::Database>("app.db");
auto user_repo = std::make_shared<sohbet::repositories::UserRepository>(db);
sohbet::helpers::UserHelpers helpers(user_repo);

auto user = helpers.createUser(
    "john_doe",               // username
    "john@example.com",       // email
    "SecurePassword123",      // password
    "John Doe",              // full name
    "MIT",                   // university
    "Computer Science",      // department
    2023                     // enrollment year
);

if (user.has_value()) {
    std::cout << "User created with ID: " << user->getId().value() << std::endl;
} else {
    std::cerr << "Error: " << helpers.getLastError() << std::endl;
}
```

### Authenticating a User (e.g., login form)

```cpp
auto authenticated = helpers.authenticateUser("john_doe", "SecurePassword123");

if (authenticated.has_value()) {
    std::cout << "Welcome, " << authenticated->getName().value() << "!" << std::endl;
    // Generate session token, redirect to dashboard, etc.
} else {
    std::cerr << "Login failed: " << helpers.getLastError() << std::endl;
}
```

### Updating User Profile

```cpp
bool success = helpers.updateUser(
    user_id,
    "John A. Doe",          // updated name
    std::nullopt,           // position unchanged
    "+1-555-0123",          // new phone
    std::nullopt,           // university unchanged
    std::nullopt,           // department unchanged
    std::nullopt,           // enrollment year unchanged
    "English"               // new primary language
);
```

### Application Startup with Migrations

```cpp
int main() {
    // Initialize database with automatic migrations
    sohbet::init::DatabaseInitializer initializer("production.db", "migrations");
    
    if (!initializer.initialize()) {
        std::cerr << "Failed to initialize: " << initializer.getLastError() << std::endl;
        return 1;
    }
    
    // Database is ready, schema is up-to-date
    auto db = initializer.getDatabase();
    
    // Continue with application setup...
}
```

## Testing

### Running Tests

```bash
cd build
ctest --output-on-failure
```

### Test Coverage

- **test_user_helpers.cpp**: Tests all user helper functions
  - User creation with validation
  - Duplicate user detection
  - User retrieval
  - Profile updates
  - Authentication with correct/incorrect passwords

- **test_migration_runner.cpp**: Tests migration system
  - Migration initialization
  - Migration registration
  - Migration application
  - Pending migration detection
  - Loading from directory

All tests pass (11/11 total tests in the project).

## Example Program

**Location:** `examples/user_management_example.cpp`

A comprehensive example demonstrating:
1. Database initialization with migrations
2. User registration from form data
3. User authentication (login)
4. Failed authentication (wrong password)
5. Profile updates
6. User information retrieval
7. Duplicate user handling

**Running the example:**
```bash
./build/user_management_example
```

## Integration with Existing Code

The implementation integrates seamlessly with existing code:

- Uses existing `UserRepository` for database operations
- Uses existing `bcrypt_wrapper` for password hashing
- Uses existing `Database` class for SQLite operations
- Follows existing code patterns and naming conventions
- No breaking changes to existing functionality

## Security

- Passwords are hashed using bcrypt (already in use)
- No passwords stored in plain text
- Input validation on all user inputs
- SQL injection prevention via prepared statements
- No security vulnerabilities found by CodeQL

## Files Added

```
include/helpers/user_helpers.h
src/helpers/user_helpers.cpp
include/db/migration_runner.h
src/db/migration_runner.cpp
include/init/database_initializer.h
src/init/database_initializer.cpp
migrations/000_create_users.sql
tests/test_user_helpers.cpp
tests/test_migration_runner.cpp
examples/user_management_example.cpp
```

## Files Modified

```
CMakeLists.txt (added new source files and tests)
```

## Future Enhancements

Potential improvements for future consideration:

1. Migration rollback support
2. Migration dry-run mode
3. User password reset functionality
4. Batch user operations
5. User search/filtering helpers
6. Email verification helpers
7. Database backup before migrations

## Conclusion

This implementation provides a solid foundation for user management and database schema evolution in the Sohbet project. The high-level helper functions simplify common user operations while the migration system ensures smooth database schema updates as the application evolves.
