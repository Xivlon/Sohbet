# Deployment Guide for Sohbet

This guide provides instructions for deploying Sohbet in production environments, including the necessary environment variable configurations.

## Current Production Deployments

- **Backend (Fly.io)**: https://sohbet-uezxqq.fly.dev
- **Frontend (Vercel)**: https://sohbet-seven.vercel.app/
- **JWT Secret**: yDE0NNfNuaOt6/9aVK6D3bRW6yHYqhjVypiaVZai0Hg=

## Environment Variables

### Frontend (Vercel/Next.js)

Create a `.env.local` file in the `frontend/` directory with the following variables:

```env
# Production Backend URLs - Actual deployment
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

**Important Notes:**
- `NEXT_PUBLIC_API_URL`: The base URL for your backend API (must use `https://` for production)
- `NEXT_PUBLIC_WS_URL`: The WebSocket URL (must use `wss://` for secure connections on HTTPS sites)
- Do NOT commit `.env.local` to version control (it's in `.gitignore`)
- For local development, you can use `http://localhost:8080` and `ws://localhost:8081`

#### Vercel Deployment

In your Vercel project settings, add these environment variables:

```
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

### Backend (Fly.io/C++)

Set the following environment variables on your backend deployment:

```bash
# On Fly.io
fly secrets set SOHBET_JWT_SECRET=yDE0NNfNuaOt6/9aVK6D3bRW6yHYqhjVypiaVZai0Hg=
fly secrets set CORS_ORIGIN=https://sohbet-seven.vercel.app
fly secrets set PORT=8080
fly secrets set WS_PORT=8081
```

**Environment Variables:**
- `SOHBET_JWT_SECRET`: Secret key for JWT token generation (required)
- `CORS_ORIGIN`: Allowed origin for CORS requests (e.g., your frontend URL). Use `*` for development to allow all origins
- `PORT`: HTTP server port (default: 8080)
- `WS_PORT`: WebSocket server port (default: 8081)

#### Fly.io Configuration for WebSocket TLS

**Important**: The `backend/fly.toml` file must be configured correctly to handle WebSocket connections. Due to how Fly.io's HTTP proxy works, WebSocket connections require special configuration to avoid handshake failures.

**Correct Configuration** (uses TCP protocol mode):

```toml
# WebSocket Service (port 8081)
# Using TCP protocol to avoid HTTP proxy interference with WebSocket upgrade
[[services]]
  internal_port = 8081
  processes = ['app']
  protocol = "tcp"  # CRITICAL: Use TCP mode, not HTTP mode
  auto_stop_machines = 'stop'
  auto_start_machines = true
  min_machines_running = 0

  [[services.ports]]
    port = 8081
    handlers = ["tls"]  # TLS only, no HTTP handler
```

**Why TCP protocol?**
- Fly.io's HTTP proxy expects complete HTTP responses and doesn't handle WebSocket upgrade (HTTP 101) properly
- Using `protocol = "tcp"` tells Fly.io to pass through raw TCP after TLS termination
- This allows the WebSocket handshake to complete without proxy interference
- Still provides TLS encryption for secure WebSocket (`wss://`) connections

**Common mistake**: Using `handlers = ["tls", "http"]` causes `[PU02]` errors because the HTTP proxy closes connections on WebSocket upgrade responses.

For more details, see [WEBSOCKET_HANDSHAKE_FIX.md](WEBSOCKET_HANDSHAKE_FIX.md).

## Security Considerations

### Protocol Mismatch Prevention

The frontend now includes validation to prevent insecure WebSocket connections (`ws://`) from HTTPS pages. If you attempt to connect to a non-secure WebSocket from an HTTPS page, you'll see an error message instructing you to configure the `NEXT_PUBLIC_WS_URL` environment variable.

### CORS Configuration

The backend server's CORS configuration is controlled by the `CORS_ORIGIN` environment variable:
- **Production**: Set to your frontend domain (e.g., `https://sohbet-seven.vercel.app`)
- **Development**: Can be set to `*` to allow all origins
- **Default**: If not set, defaults to `*` (not recommended for production)

## Deployment Checklist

### Frontend Deployment (Vercel)

- [ ] Set `NEXT_PUBLIC_API_URL` environment variable in Vercel
- [ ] Set `NEXT_PUBLIC_WS_URL` environment variable in Vercel
- [ ] Verify environment variables use `https://` and `wss://` protocols
- [ ] Deploy frontend
- [ ] Test API connectivity in browser console
- [ ] Verify WebSocket connection establishes successfully

### Backend Deployment (Fly.io)

- [ ] Set `SOHBET_JWT_SECRET` secret
- [ ] Set `CORS_ORIGIN` to frontend domain
- [ ] Optional: Set `PORT` and `WS_PORT` if using non-default ports
- [ ] Deploy backend
- [ ] Verify `/api/status` endpoint is accessible
- [ ] Check WebSocket port is accessible (not blocked by firewall)

## Testing Your Deployment

### 1. Test API Connection

Open your frontend in a browser and check the console:
```javascript
// Should show successful API request
fetch('https://sohbet-uezxqq.fly.dev/api/status')
  .then(r => r.json())
  .then(console.log)
```

### 2. Test WebSocket Connection

In the browser console:
```javascript
// Should establish connection without errors
const ws = new WebSocket('wss://sohbet-uezxqq.fly.dev/?token=test-token');
ws.onopen = () => console.log('✓ WebSocket connected');
ws.onerror = (e) => console.error('✗ WebSocket error:', e);
```

### 3. Verify Browser DevTools

- **Network tab**: Should show successful API requests (status 200/201)
- **Console**: No CORS errors
- **Console**: WebSocket connection shows as "connected"
- **Application tab**: Auth token stored in localStorage

## Troubleshooting

### CORS Errors

If you see CORS errors in the browser console:
1. Verify `CORS_ORIGIN` is set on the backend to your frontend URL
2. Ensure there's no trailing slash in the CORS_ORIGIN value
3. Restart the backend service after changing environment variables

### WebSocket Connection Failures

If WebSocket fails to connect with errors like "Firefox can't establish a connection to the server at wss://...":

1. **Verify TCP protocol configuration** in `backend/fly.toml`:
   - Port 8081 MUST use `protocol = "tcp"` with `handlers = ["tls"]` only
   - Do NOT use `handlers = ["tls", "http"]` as this causes handshake failures
   - See [WEBSOCKET_HANDSHAKE_FIX.md](WEBSOCKET_HANDSHAKE_FIX.md) for details
   
2. **Check environment variables**:
   - Verify `NEXT_PUBLIC_WS_URL` uses `wss://` (not `ws://`) for HTTPS sites
   - Ensure the WebSocket URL includes the correct port (`:8081`)
   - Example: `wss://sohbet-uezxqq.fly.dev:8081`

3. **Verify network connectivity**:
   - Check that the WebSocket port (8081) is not blocked by firewall
   - Ensure the backend WebSocket server is running
   - Test the port is accessible: `nc -zv your-backend.fly.dev 8081`

4. **Check browser console** for specific error messages:
   - "Protocol mismatch" errors indicate trying to use `ws://` from HTTPS page
   - "Connection refused" may indicate backend not running or port blocked
   - Check the browser console for reconnection attempts and error details

### API 404 Errors

If API requests return 404:
1. Verify `NEXT_PUBLIC_API_URL` is correctly set
2. Check that the URL doesn't have duplicate slashes or trailing slashes
3. Ensure the backend is deployed and accessible
4. Test the `/api/status` endpoint directly in a browser

### Missing Environment Variables

If you see "API base URL not configured" errors:
1. Ensure environment variables are set in your deployment platform
2. For Vercel, environment variables must start with `NEXT_PUBLIC_` to be accessible in the browser
3. Rebuild and redeploy after setting environment variables

## Local Development

For local development, create `frontend/.env.local`:

```env
NEXT_PUBLIC_API_URL=http://localhost:8080
NEXT_PUBLIC_WS_URL=ws://localhost:8081
```

Then run:
```bash
# Terminal 1: Backend
cd /path/to/Sohbet
mkdir build && cd build
cmake ..
make
./sohbet

# Terminal 2: Frontend
cd /path/to/Sohbet/frontend
npm install
npm run dev
```

Access the application at `http://localhost:5000`

## Additional Resources

- [Vercel Environment Variables](https://vercel.com/docs/environment-variables)
- [Fly.io Secrets Management](https://fly.io/docs/reference/secrets/)
- [Next.js Environment Variables](https://nextjs.org/docs/basic-features/environment-variables)
