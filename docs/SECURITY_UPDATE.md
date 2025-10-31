# Security Update - JWT Secret Configuration

## 🔐 Critical Security Fix Applied

**Date:** October 27, 2025  
**Issue:** Hardcoded JWT secret vulnerability  
**Status:** ✅ FIXED

---

## What Changed

### Before (Vulnerable):
```cpp
// Hardcoded default secret - ANYONE could forge tokens!
std::string generate_jwt_token(..., const std::string& secret = "default_secret");
```

### After (Secure):
```cpp
// Secret MUST be provided from environment variable
std::string jwt_secret = config::get_jwt_secret();  // Reads from SOHBET_JWT_SECRET
std::string token = generate_jwt_token(username, user_id, role, jwt_secret, expiry);
```

---

## Required Action: Set Environment Variable

### For Development

Create a `.env` file or set the environment variable:

```bash
export SOHBET_JWT_SECRET="your-super-secret-key-here"
```

Or generate a secure random secret:

```bash
# Generate a strong random secret (recommended)
export SOHBET_JWT_SECRET=$(openssl rand -base64 32)
```

### For Production (Replit)

1. Go to the **Secrets** tab in Replit
2. Add a new secret:
   - **Key:** `SOHBET_JWT_SECRET`
   - **Value:** A strong random string (use the openssl command above)
3. Restart the backend server

---

## Files Modified

### New Files:
- `include/config/env.h` - Environment variable configuration utility
- `.env.example` - Example environment variables template
- `SECURITY_UPDATE.md` - This documentation

### Updated Files:
- `include/security/jwt.h` - Removed hardcoded default secret
- `src/server/server.cpp` - Updated to use config::get_jwt_secret()
- `src/services/permission_service.cpp` - Updated all JWT verification calls
- `src/server/websocket_server.cpp` - Updated WebSocket authentication

---

## Error Handling

If the environment variable is not set, the server will throw an error on startup:

```
CRITICAL: JWT secret not configured. 
Set SOHBET_JWT_SECRET environment variable before starting the server.
```

This is intentional - the server will **NOT start** without a proper JWT secret configured.

---

## Additional Configuration Options

### JWT Expiry Time

Set the token expiration time (in hours):

```bash
export JWT_EXPIRY_HOURS=24  # Default is 24 hours
```

### Alternative Secret Name

For compatibility, the system also checks for `JWT_SECRET` as a fallback:

```bash
export JWT_SECRET="your-secret-key"
```

---

## Security Best Practices

1. **Never commit secrets to git** - Add `.env` to `.gitignore`
2. **Use different secrets for dev/prod** - Development and production should have different secrets
3. **Rotate secrets periodically** - Change the JWT secret every 6-12 months
4. **Use strong random secrets** - Minimum 32 characters, generated randomly
5. **Keep secrets in secure storage** - Use Replit Secrets, not hardcoded values

---

## Testing the Fix

### 1. Start the backend without the secret:
```bash
cd build && ./sohbet
```
**Expected:** Error message about missing SOHBET_JWT_SECRET

### 2. Start with the secret:
```bash
export SOHBET_JWT_SECRET="test-secret-for-development"
cd build && ./sohbet
```
**Expected:** Server starts successfully

### 3. Test login:
```bash
curl -X POST http://0.0.0.0:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"demo_student","password":"demo123"}'
```
**Expected:** Returns a valid JWT token

---

## Migration Notes

**Breaking Change:** The backend will no longer start without a JWT secret configured.

**Action Required:** All developers and deployment environments MUST set the `SOHBET_JWT_SECRET` environment variable before starting the server.

---

## Questions?

For more information about JWT best practices, see:
- [JWT.io](https://jwt.io/)
- [OWASP JWT Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/JSON_Web_Token_for_Java_Cheat_Sheet.html)
