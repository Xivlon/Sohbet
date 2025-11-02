# Frontend Deployment Guide

This document provides comprehensive instructions for deploying the Sohbet frontend to various platforms, with special focus on integration with the C++ backend hosted on fly.io.

## Backend Configuration

The Sohbet backend is deployed on fly.io and provides the following endpoints:

- **HTTP API**: `https://sohbet-uezxqq.fly.dev`
- **WebSocket**: `wss://sohbet-uezxqq.fly.dev:8081`

## Environment Variables

The frontend requires two environment variables to connect to the backend:

### Production (Vercel/Netlify)

```env
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

### Local Development

```env
NEXT_PUBLIC_API_URL=http://localhost:8080
NEXT_PUBLIC_WS_URL=ws://localhost:8081
```

## Vercel Deployment

### Option 1: Deploy from Root (Recommended)

If deploying the entire monorepo to Vercel:

1. **Project Settings**:
   - Framework Preset: `Next.js`
   - Root Directory: `frontend`
   - Build Command: `npm run build` (default)
   - Output Directory: `.next` (default)

2. **Environment Variables** (in Vercel Dashboard):
   ```
   NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
   NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
   ```

3. **Deploy**:
   ```bash
   # Push to your repository
   git push origin main
   ```

### Option 2: Deploy Frontend Directory Only

If deploying only the frontend directory:

1. Navigate to `frontend` directory
2. Create `.env.local`:
   ```env
   NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
   NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
   ```
3. Deploy:
   ```bash
   vercel
   ```

### Vercel Configuration Files

The repository includes two `vercel.json` files:

- **Root `vercel.json`**: Contains environment variables for monorepo deployment
- **`frontend/vercel.json`**: Contains environment variables for frontend-only deployment

Both are configured with the correct production URLs.

## Netlify Deployment

### Configuration

Create `netlify.toml` in the frontend directory:

```toml
[build]
  base = "frontend"
  command = "npm run build"
  publish = ".next"

[build.environment]
  NEXT_PUBLIC_API_URL = "https://sohbet-uezxqq.fly.dev"
  NEXT_PUBLIC_WS_URL = "wss://sohbet-uezxqq.fly.dev:8081"
```

### Deploy

```bash
netlify deploy --prod
```

## API Integration

### API Client Configuration

The `api-client.ts` automatically uses the environment variables:

```typescript
const API_BASE_URL = (API_URL || 'http://localhost:8080').replace(/\/+$/, '');
```

### WebSocket Configuration

The `websocket-service.ts` automatically handles URL conversion:

```typescript
const envWsUrl = process.env.NEXT_PUBLIC_WS_URL;
// Automatically converts https:// to wss://
this.url = envWsUrl.replace(/^https:\/\//, 'wss://').replace(/^http:\/\//, 'ws://');
```

## Error Handling

### Authentication Errors (401)

The frontend handles 401 errors by:
1. Clearing the authentication token from localStorage
2. Redirecting to the login page
3. Displaying an appropriate error message

### Network Errors

Network errors are caught and displayed with user-friendly messages:
- Connection refused: "Cannot connect to backend server"
- Timeout: "Request timed out"
- General errors: Error message from the API

### CORS Errors

If you encounter CORS errors, ensure the backend's `CORS_ORIGIN` is set correctly:

```bash
fly secrets set CORS_ORIGIN=https://your-frontend-domain.vercel.app
```

## Testing the Integration

### 1. Test API Connection

Open browser console and check for:
```
API Request: https://sohbet-uezxqq.fly.dev/api/status
```

### 2. Test Authentication

1. Register a new account
2. Login with the account
3. Verify the token is stored in localStorage:
   ```javascript
   localStorage.getItem('auth_token')
   ```

### 3. Test WebSocket Connection

1. Navigate to Messages/Chat
2. Check browser console for:
   ```
   WebSocket connection established to wss://sohbet-uezxqq.fly.dev:8081
   ```

## Common Issues

### Issue: 404 Errors on API Calls

**Cause**: Incorrect API URL or missing endpoints

**Solution**:
1. Verify `NEXT_PUBLIC_API_URL` is set correctly
2. Check backend is running: `curl https://sohbet-uezxqq.fly.dev/api/status`
3. Verify the endpoint exists in the backend

### Issue: 401 Errors

**Cause**: Missing or invalid authentication token

**Solution**:
1. Clear localStorage and login again
2. Verify token is being sent in Authorization header
3. Check token is not expired

### Issue: 403 Errors

**Cause**: Insufficient permissions

**Solution**:
1. Verify user has the correct role
2. Check backend permission configuration
3. Ensure the user is authenticated

### Issue: WebSocket Connection Failed

**Cause**: Incorrect WebSocket URL or firewall blocking

**Solution**:
1. Verify `NEXT_PUBLIC_WS_URL` is set to `wss://sohbet-uezxqq.fly.dev:8081`
2. Check fly.io configuration includes WebSocket service on port 8081
3. Test WebSocket connection manually:
   ```javascript
   const ws = new WebSocket('wss://sohbet-uezxqq.fly.dev:8081');
   ws.onopen = () => console.log('Connected');
   ```

## Build Verification

Before deploying, verify the build succeeds locally:

```bash
cd frontend
npm install
npm run build
```

Expected output:
```
✓ Compiled successfully
✓ Generating static pages
Route (app)
├ ƒ /
├ ƒ /friends
├ ƒ /groups
├ ƒ /messages
└ ƒ /organizations
```

## TypeScript Configuration

The frontend uses strict TypeScript with comprehensive interfaces for all API responses:

- `User`, `Post`, `Comment`, `Group`, `Organization`
- `Conversation`, `Message`, `Friendship`, `Reaction`
- `VoiceChannel`, `VoiceConnectionToken`, `Media`

All API methods are fully typed, providing IntelliSense and compile-time type checking.

## Security Considerations

1. **Never commit `.env.local`** - It's in `.gitignore`
2. **Use environment variables** - Never hardcode API URLs
3. **HTTPS/WSS in production** - Always use secure protocols
4. **Token storage** - Tokens are stored in localStorage (consider httpOnly cookies for enhanced security)
5. **CORS configuration** - Backend must allow your frontend domain

## Support

For issues related to:
- **Frontend deployment**: Check this guide and Vercel/Netlify documentation
- **Backend integration**: See `docs/DEPLOYMENT_GUIDE.md`
- **WebSocket issues**: See `docs/WEBSOCKET_HANDSHAKE_FIX.md`
- **API issues**: Check backend logs on fly.io

## Next Steps

After successful deployment:

1. ✅ Verify all pages load correctly
2. ✅ Test authentication flow
3. ✅ Test all major features (posts, groups, messages)
4. ✅ Monitor for errors in browser console
5. ✅ Check backend logs for any 404/401/403 errors

---

**Last Updated**: November 2, 2025
**Backend Version**: 0.3.0-academic
**Frontend Framework**: Next.js 16.0.0
