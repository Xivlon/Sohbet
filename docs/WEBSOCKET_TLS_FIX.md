# WebSocket TLS Connection Fix

## Problem Statement

The WebSocket connection from the frontend (deployed on Vercel via HTTPS) to the backend (deployed on Fly.io) was failing with the error:

```
Firefox can't establish a connection to the server at wss://sohbet-uezxqq.fly.dev:8081
```

Users experienced continuous reconnection attempts (5 attempts with exponential backoff) but were unable to establish a connection for real-time features like chat, typing indicators, and online presence.

## Root Cause Analysis

The issue was caused by **missing TLS/HTTPS handlers** on the WebSocket port (8081) in the Fly.io configuration.

### Technical Details

1. **Frontend Context**: The frontend is deployed on Vercel and served over HTTPS (`https://sohbet-seven.vercel.app`)
2. **Browser Security**: Modern browsers (Firefox, Chrome, Safari) **block insecure WebSocket connections** (`ws://`) from secure pages (`https://`)
3. **Backend Configuration**: The Fly.io `fly.toml` had port 8081 configured with only HTTP handlers: `handlers = ["http"]`
4. **Result**: When the frontend tried to connect via `wss://` (WebSocket Secure), the connection failed because port 8081 wasn't configured to handle TLS/HTTPS traffic

### Why This Matters

- **HTTP port (8080)** was correctly configured with both HTTP and TLS handlers: `handlers = ["tls", "http"]`
- **WebSocket port (8081)** was only configured with HTTP handlers: `handlers = ["http"]`
- This asymmetry meant API calls worked fine (HTTPS on port 443 → HTTP on port 8080), but WebSocket connections failed (WSS on port 8081 → no TLS support)

## Solution Implemented

### 1. Updated Fly.io Configuration (`backend/fly.toml`)

**Before:**
```toml
[[services.ports]]
  port = 8081
  handlers = ["http"]
```

**After:**
```toml
[[services.ports]]
  port = 8081
  handlers = ["tls", "http"]  # Added TLS support
```

This change enables Fly.io to:
- Accept secure WebSocket connections (`wss://`) on port 8081
- Automatically handle TLS termination
- Route the decrypted traffic to the internal WebSocket server on port 8081

### 2. Updated Vercel Configuration (`frontend/vercel.json`)

**Before:**
```json
{
  "env": {
    "NEXT_PUBLIC_API_URL": "https://sohbet-uezxqq.fly.dev"
  }
}
```

**After:**
```json
{
  "env": {
    "NEXT_PUBLIC_API_URL": "https://sohbet-uezxqq.fly.dev",
    "NEXT_PUBLIC_WS_URL": "wss://sohbet-uezxqq.fly.dev:8081"
  }
}
```

This ensures the frontend uses the correct WebSocket URL with:
- Secure protocol (`wss://`)
- Correct hostname
- Explicit port number (`:8081`)

### 3. Updated Documentation (`docs/DEPLOYMENT_GUIDE.md`)

Added detailed section on:
- WebSocket TLS configuration requirements
- Troubleshooting WebSocket connection failures
- Step-by-step verification procedures
- Common error messages and their solutions

## Deployment Instructions

### For Repository Maintainers

To deploy this fix:

1. **Deploy Backend Changes**:
   ```bash
   cd backend
   fly deploy
   ```
   
   This will apply the updated `fly.toml` configuration with TLS handlers on port 8081.

2. **Redeploy Frontend**:
   - The frontend will automatically redeploy from Vercel when you merge this PR
   - The new `NEXT_PUBLIC_WS_URL` environment variable will be available
   - No manual configuration needed if using the updated `vercel.json`

3. **Verify the Fix**:
   ```bash
   # Test WebSocket port is accessible with TLS
   openssl s_client -connect sohbet-uezxqq.fly.dev:8081 -servername sohbet-uezxqq.fly.dev
   ```
   
   Expected output: Should successfully connect and show TLS certificate details

### For Users Setting Up New Deployments

If deploying to a new environment:

1. **Backend (Fly.io)**:
   - Ensure `backend/fly.toml` has `handlers = ["tls", "http"]` for port 8081
   - Deploy with `fly deploy`

2. **Frontend (Vercel)**:
   - Set environment variables in Vercel dashboard:
     - `NEXT_PUBLIC_API_URL`: `https://your-backend.fly.dev`
     - `NEXT_PUBLIC_WS_URL`: `wss://your-backend.fly.dev:8081`
   - Or use `vercel.json` with these values

## Testing & Verification

### 1. Check TLS Configuration

```bash
# Test port 8081 accepts TLS connections
curl -v --insecure wss://sohbet-uezxqq.fly.dev:8081
```

Expected: Connection established (will fail auth, but connection works)

### 2. Browser Console Test

Open browser DevTools on the frontend and check:

```javascript
// Should show successful connection
const ws = new WebSocket('wss://sohbet-uezxqq.fly.dev:8081/?token=test-token');
ws.onopen = () => console.log('✓ WebSocket connected');
ws.onerror = (e) => console.error('✗ WebSocket error:', e);
```

Expected: "✓ WebSocket connected" message

### 3. Real-Time Features Test

After logging in to the application:
- [COMPLETE] Chat messages should send/receive in real-time
- [COMPLETE] Typing indicators should appear when someone is typing
- [COMPLETE] Online/offline status should update immediately
- [COMPLETE] No reconnection attempts in console

## Common Issues & Solutions

### Issue: Still seeing "can't establish connection" errors

**Solution**: 
1. Verify backend was redeployed: `fly status`
2. Check TLS is enabled: `fly config show` should show handlers for port 8081
3. Restart the app: `fly apps restart sohbet-uezxqq`

### Issue: "Protocol mismatch" errors

**Solution**: Ensure frontend is using `wss://` not `ws://`
- Check browser console for the actual URL being used
- Verify `NEXT_PUBLIC_WS_URL` is set correctly
- Rebuild and redeploy frontend

### Issue: Connection works locally but not in production

**Solution**: 
- Local development uses `ws://` which is allowed from `http://`
- Production must use `wss://` from `https://`
- Ensure environment variables are different for dev vs. prod

## Technical Background

### WebSocket Protocol Security

- **ws://** - Insecure WebSocket (like HTTP)
- **wss://** - Secure WebSocket (like HTTPS)

Browsers enforce:
- `http://` pages can use `ws://` connections
- `https://` pages **must** use `wss://` connections
- Mixed content is blocked for security

### Fly.io TLS Handling

When you add `"tls"` to handlers:
1. Fly.io's edge proxy terminates TLS connections
2. Issues/validates SSL certificates automatically
3. Forwards decrypted traffic to your internal port
4. Handles SSL certificate renewal

This is why the fix is simple - just add `"tls"` to the handlers array!

## Files Changed

```
backend/fly.toml              | 1 line changed
frontend/vercel.json          | 2 lines added
docs/DEPLOYMENT_GUIDE.md      | 38 lines added
docs/WEBSOCKET_TLS_FIX.md     | New file
```

## References

- [Fly.io Services Configuration](https://fly.io/docs/reference/configuration/#the-services-sections)
- [MDN: WebSocket Security](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers#security)
- [RFC 6455: The WebSocket Protocol](https://tools.ietf.org/html/rfc6455)
- [OWASP: WebSocket Security](https://owasp.org/www-community/vulnerabilities/WebSocket_security)

## Success Metrics

After deployment, you should see:
- [COMPLETE] Zero WebSocket connection errors in browser console
- [COMPLETE] Zero reconnection attempts
- [COMPLETE] Real-time chat messages appear instantly (< 100ms latency)
- [COMPLETE] Typing indicators work smoothly
- [COMPLETE] Online status updates immediately

---

**Status**: [COMPLETE] Fix implemented and ready for deployment
**Priority**: High - Blocks all real-time features
**Effort**: Minimal - Configuration change only
