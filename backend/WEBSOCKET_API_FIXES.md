# WebSocket and API Endpoint Fixes - Summary

## Overview
This PR fixes critical issues preventing the Sohbet application from working correctly in production environments where the frontend is served over HTTPS (Vercel) and the backend is deployed separately (Fly.io).

## Problems Fixed

### 1. WebSocket Protocol Mismatch ✅
**Problem**: Frontend served over HTTPS was attempting to connect via insecure `ws://` protocol, which modern browsers block.

**Solution**: 
- WebSocket service now uses `NEXT_PUBLIC_WS_URL` environment variable
- Automatically converts `https://` to `wss://` for secure connections
- Added validation to prevent insecure connections from HTTPS pages
- Provides clear error messages when misconfigured

**Files Changed**:
- `frontend/app/lib/websocket-service.ts`

### 2. Missing Environment Configuration ✅
**Problem**: Frontend lacked environment variables for production backend URLs.

**Solution**:
- Created `.env.local` template with production URLs
- Created `.env.example` to guide users
- Added comprehensive deployment documentation

**Files Created**:
- `frontend/.env.local` (gitignored)
- `frontend/.env.example`
- `docs/DEPLOYMENT_GUIDE.md`

### 3. API Base URL Hardcoded to Localhost ✅
**Problem**: `api-client.ts` defaulted to `http://0.0.0.0:8080` instead of using production backend.

**Solution**:
- API client now validates `NEXT_PUBLIC_API_URL` is configured
- Added better error logging showing attempted endpoint
- Removed hardcoded localhost fallback

**Files Changed**:
- `frontend/app/lib/api-client.ts`

### 4. Auth Utils Fallback to Window Location ✅
**Problem**: `auth.ts` would fallback to `window.location.origin`, causing API calls to point to the frontend instead of backend.

**Solution**:
- Removed fallback to `window.location`
- Now requires proper `NEXT_PUBLIC_API_URL` configuration
- Provides clear error message when not configured

**Files Changed**:
- `frontend/src/utils/auth.ts`

### 5. CORS Configuration Not Flexible ✅
**Problem**: Backend CORS headers were hardcoded to `*`, not suitable for production.

**Solution**:
- Added `CORS_ORIGIN` environment variable support
- Backend now reads `CORS_ORIGIN` for allowed origins
- Defaults to `*` for development if not set
- Added `Access-Control-Allow-Credentials: true` header

**Files Changed**:
- `src/server/server.cpp`
- `include/config/env.h`

## Environment Variables

### Frontend (Vercel)
```env
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

### Backend (Fly.io)
```bash
SOHBET_JWT_SECRET=<your-secret>
CORS_ORIGIN=https://sohbet-henna.vercel.app
PORT=8080  # optional, defaults to 8080
WS_PORT=8081  # optional, defaults to 8081
```

## Testing

### Build Tests
- ✅ Frontend builds successfully with Next.js 16.0.0
- ✅ Backend compiles without errors
- ✅ All C++ tests pass (authentication, config, etc.)

### Manual Testing Needed
After deployment, verify:
1. Browser DevTools Network tab shows successful API requests
2. WebSocket connection establishes over `wss://`
3. Real-time messages work without page refresh
4. Typing indicators show up
5. Online/offline status updates
6. No console errors about insecure connections
7. No CORS errors

## Documentation

### New Files
- **docs/DEPLOYMENT_GUIDE.md**: Comprehensive deployment instructions
  - Environment variable setup for Vercel and Fly.io
  - Security considerations
  - Deployment checklist
  - Testing procedures
  - Troubleshooting guide

- **frontend/.env.example**: Template for environment variables

### Updated Files
- **README.md**: Added reference to deployment guide

## Security Improvements

1. **Protocol Enforcement**: Prevents insecure WebSocket connections from HTTPS pages
2. **CORS Configuration**: Allows restricting origins in production
3. **No Fallbacks**: Removed dangerous fallback to window.location
4. **Validation**: All environment variables validated before use

## Breaking Changes

⚠️ **Important**: After deploying this PR, you MUST set environment variables:

### Frontend (Vercel)
Set these in your Vercel project settings:
- `NEXT_PUBLIC_API_URL`
- `NEXT_PUBLIC_WS_URL`

### Backend (Fly.io)
Set these secrets:
```bash
fly secrets set CORS_ORIGIN=https://sohbet-henna.vercel.app
fly secrets set SOHBET_JWT_SECRET=<your-secret>
```

Without these, the application will not work!

## Deployment Instructions

1. **Deploy Backend First**:
   ```bash
   fly secrets set CORS_ORIGIN=https://sohbet-henna.vercel.app
   fly secrets set SOHBET_JWT_SECRET=<your-secret>
   fly deploy
   ```

2. **Deploy Frontend**:
   - Set environment variables in Vercel dashboard
   - Redeploy frontend

3. **Verify**:
   - Test `/api/status` endpoint
   - Check WebSocket connection
   - Test login and real-time features

## Additional Notes

- `.env.local` is properly gitignored
- Environment variables are validated at runtime
- Clear error messages guide users to fix configuration issues
- Documentation provides complete deployment workflow

## Files Changed Summary

```
docs/DEPLOYMENT_GUIDE.md              | 174 +++++++++++++++++++
README.md                             |   8 +
frontend/.env.example                 |  11 ++
frontend/app/lib/api-client.ts        |  14 +-
frontend/app/lib/websocket-service.ts |  33 +++-
frontend/src/utils/auth.ts            |  11 +-
include/config/env.h                  |   9 +
src/server/server.cpp                 |   6 +-
8 files changed, 255 insertions(+), 11 deletions(-)
```

## References

- [Deployment Guide](../docs/DEPLOYMENT_GUIDE.md)
- [Vercel Environment Variables](https://vercel.com/docs/environment-variables)
- [Fly.io Secrets](https://fly.io/docs/reference/secrets/)
