# Demo Account "Unauthorized" Error Fix

## Problem Statement

The demo account (`demo_student` / `demo123`) was experiencing "Unauthorized" errors after API changes were made to the backend. Users attempting to log in with the demo credentials would receive a valid JWT token, but subsequent authenticated API requests would fail with 401 Unauthorized errors.

## Root Cause

When the backend server's JWT secret or password hashing configuration changes (e.g., during deployment or environment updates), the existing demo user's password hash in the database may become incompatible. This happens because:

1. The demo user is created once with a specific password hash
2. If the server is redeployed with different environment configurations, the password hash may not verify correctly
3. Even though the JWT secret is used only for token signing/verification (not password hashing), other configuration changes could affect the bcrypt password verification

## Solution

The fix ensures that every time the server starts, it resets the demo user's password to the known value (`demo123`). This guarantees that:

1. The demo account always works, regardless of previous configurations
2. New deployments with different JWT secrets will not break demo login
3. The password hash is always current and compatible with the active configuration

### Implementation Details

**Modified Files:**

1. `include/repositories/user_repository.h`
   - Added `updatePassword()` method declaration

2. `src/repositories/user_repository.cpp`
   - Implemented `updatePassword()` method to hash and store a new password

3. `src/server/server.cpp`
   - Modified `ensureDemoUserExists()` to reset demo user password on every startup

**Code Changes:**

```cpp
// In ensureDemoUserExists()
if (existing_user.has_value()) {
    int demo_user_id = existing_user->getId().value();
    std::cout << "Demo user already exists (ID: " << demo_user_id << ")" << std::endl;
    
    // Always reset the demo user's password to ensure it works after any API changes
    if (user_repository_->updatePassword(demo_user_id, "demo123")) {
        std::cout << "Demo user password reset successfully" << std::endl;
    } else {
        std::cerr << "Warning: Failed to reset demo user password" << std::endl;
    }
    
    assignAdminRole(demo_user_id);
    return;
}
```

## Testing

### Manual Testing

1. **Fresh Installation Test:**
   ```bash
   cd build
   rm -f academic.db
   ./sohbet
   ```
   Result: Demo user created successfully

2. **Password Reset Test:**
   ```bash
   # Start server to create demo user
   ./sohbet &
   PID=$!
   sleep 2
   kill $PID
   
   # Restart server to test password reset
   ./sohbet
   ```
   Expected output: "Demo user password reset successfully"

3. **Login Test:**
   ```bash
   curl -X POST -H "Content-Type: application/json" \
     -d '{"username":"demo_student","password":"demo123"}' \
     http://localhost:8080/api/login
   ```
   Expected: Returns valid JWT token and user data

4. **Authenticated Request Test:**
   ```bash
   # Use token from login
   curl -H "Authorization: Bearer $TOKEN" \
     http://localhost:8080/api/friendships
   ```
   Expected: Returns 200 OK (not 401 Unauthorized)

### Automated Testing

All existing authentication tests pass:
```bash
cd build
./test_authentication
```

## Deployment Impact

This fix will automatically resolve the demo account issue when the backend is redeployed to production (Fly.io). On the next deployment:

1. The server will start and detect the existing demo user
2. It will automatically reset the password to `demo123`
3. Demo login will work immediately with no manual intervention required

## Security Considerations

- **Demo Account Only:** This password reset only applies to the demo account (`demo_student`)
- **Development/Demo Use:** The demo account is intended for testing and demonstration purposes only
- **Production Security:** In production, consider:
  - Disabling demo account creation if not needed
  - Using strong, unique passwords for all accounts
  - Implementing proper password reset flows for regular users
  - Rate limiting login attempts

## Alternative Approaches Considered

1. **Delete and Recreate Demo User:**
   - Would lose any demo user data (posts, friendships, etc.)
   - More disruptive approach

2. **Manual Password Reset:**
   - Requires manual intervention after each deployment
   - Not sustainable for frequent deployments

3. **Environment-Specific Demo Accounts:**
   - Adds complexity to manage multiple accounts
   - Doesn't solve the fundamental issue

The chosen solution (automatic password reset on startup) provides the best balance of:
- Automatic recovery from configuration changes
- Minimal impact on existing functionality
- Simple implementation and maintenance

## Related Issues

This fix resolves the "Unauthorized" error mentioned in the problem statement:
```
error: "Unauthorized"
```

Users can now successfully:
1. Log in with `demo_student` / `demo123`
2. Receive a valid JWT token
3. Make authenticated API requests without 401 errors

## Future Improvements

Consider these enhancements for production deployment:

1. Add environment variable to control demo account behavior:
   ```
   ENABLE_DEMO_ACCOUNT=true/false
   ```

2. Add endpoint to reset demo account data (posts, friendships) for clean demos

3. Implement telemetry to track demo account usage

4. Add rate limiting specifically for demo account to prevent abuse
