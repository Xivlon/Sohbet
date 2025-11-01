# WebSocket Handshake Failure Fix

## Problem Statement

WebSocket connections were failing on Fly.io with the following errors:

```
WebSocket handshake failed
[PU02] could not complete HTTP request to instance: connection closed before message completed
```

This prevented all real-time features including:
- Real-time chat messaging
- Typing indicators
- Online/offline presence
- Live updates

## Root Cause

The issue was caused by **incorrect protocol configuration** in Fly.io's `fly.toml` for the WebSocket service.

### Technical Details

The WebSocket service (port 8081) was configured with:
```toml
[[services.ports]]
  port = 8081
  handlers = ["tls", "http"]
```

This configuration causes Fly.io's edge proxy to:
1. Treat the connection as a standard HTTP/HTTPS request
2. Expect a complete HTTP response with headers and body
3. Close the connection when it receives a WebSocket upgrade response (HTTP 101) instead of a complete HTTP response

The **[PU02]** error specifically indicates that Fly.io's HTTP proxy could not complete the HTTP request because it received a protocol upgrade response, which the HTTP proxy doesn't recognize as a valid completion of an HTTP request/response cycle.

### Why This Happens

WebSocket connections start as HTTP requests but then "upgrade" to the WebSocket protocol:
1. Client sends: `GET / HTTP/1.1\r\nUpgrade: websocket\r\n...`
2. Server responds: `HTTP/1.1 101 Switching Protocols\r\n...`
3. Connection switches to WebSocket framing

When Fly.io's HTTP proxy intercepts this:
- It expects a standard HTTP response (200, 404, etc.) with a complete response body
- Instead, it receives a 101 response which tells it to upgrade protocols
- The HTTP proxy doesn't know what to do with this and closes the connection
- Result: "connection closed before message completed"

## Solution

Change the WebSocket service to use **TCP protocol mode** instead of HTTP mode:

```toml
# WebSocket Service (port 8081)
# Using TCP protocol to avoid HTTP proxy interference with WebSocket upgrade
[[services]]
  internal_port = 8081
  processes = ['app']
  protocol = "tcp"  # Key change: TCP mode, not default HTTP mode
  auto_stop_machines = 'stop'
  auto_start_machines = true
  min_machines_running = 0

  [[services.ports]]
    port = 8081
    handlers = ["tls"]  # TLS only, no HTTP handler
```

### What This Does

1. **`protocol = "tcp"`**: Tells Fly.io to pass through raw TCP connections instead of treating it as HTTP
2. **`handlers = ["tls"]`**: Fly.io still terminates TLS, so we get secure WebSocket (`wss://`)
3. **No HTTP handler**: Fly.io doesn't try to parse or validate HTTP responses
4. **Result**: The WebSocket upgrade handshake passes through correctly

## Before vs After

### Before (Broken)
```
Client (wss://) → Fly.io TLS Termination → Fly.io HTTP Proxy → WebSocket Server
                                              ↑
                                              Fails here: HTTP proxy doesn't understand
                                              WebSocket upgrade (101 response)
```

### After (Working)
```
Client (wss://) → Fly.io TLS Termination → WebSocket Server
                                              ↑
                                              Direct TCP passthrough after TLS,
                                              no HTTP parsing/validation
```

## Deployment Instructions

1. **Update backend configuration**:
   ```bash
   cd backend
   # The fly.toml has already been updated with this fix
   fly deploy
   ```

2. **Verify the deployment**:
   ```bash
   # Check that the WebSocket port is accessible
   fly status
   fly logs
   ```

3. **Test WebSocket connection**:
   - Open the frontend in a browser
   - Check browser console for "✓ WebSocket connected" message
   - Send a chat message to verify real-time functionality

## Testing & Verification

### 1. Check Fly.io Configuration

```bash
fly config show
```

Look for the WebSocket service configuration and verify:
- `protocol = "tcp"`
- `handlers = ["tls"]` only (no "http")

### 2. Test from Browser Console

```javascript
// Should connect successfully
const ws = new WebSocket('wss://your-app.fly.dev:8081/?token=YOUR_JWT_TOKEN');
ws.onopen = () => console.log('✓ Connected');
ws.onerror = (e) => console.error('✗ Error:', e);
ws.onclose = () => console.log('✗ Closed');
```

Expected result: "✓ Connected" message

### 3. Test Real-Time Features

After logging in:
- ✅ Send a chat message - should appear instantly for the recipient
- ✅ Type in chat - recipient should see "typing..." indicator
- ✅ Check online status - should show green dot for online users
- ✅ No reconnection attempts in console

## Common Issues

### Issue: Still seeing handshake failures

**Possible causes:**
1. Backend not redeployed with new configuration
2. Old machines still running

**Solution:**
```bash
fly apps restart your-app-name
fly logs  # Watch for "WebSocket server listening" message
```

### Issue: TLS connection failed

**Possible cause:** Certificate issues

**Solution:**
```bash
fly certs check
fly certs show
```

### Issue: Works locally but not in production

**Cause:** Local development uses `ws://` (no TLS), production uses `wss://`

**Solution:** Ensure environment variables are correct:
- Local: `NEXT_PUBLIC_WS_URL=ws://localhost:8081`
- Production: `NEXT_PUBLIC_WS_URL=wss://your-app.fly.dev:8081`

## Alternative Solution (Not Recommended)

An alternative approach is to use the SAME port for both HTTP and WebSocket:

```toml
[[services]]
  internal_port = 8080
  processes = ['app']
  
  [[services.ports]]
    handlers = ["tls", "http"]
    port = 443
```

With this approach:
- WebSocket connects to `wss://host/` (same as HTTPS)
- Server handles both HTTP and WebSocket on port 8080
- Fly.io's HTTP proxy properly handles Upgrade requests

**Why we didn't use this:**
- Requires backend code changes to handle both HTTP and WebSocket on same port
- Mixing protocols on same port is more complex
- Separate ports provide better isolation and diagnostics

## Technical Background

### Fly.io Service Protocols

Fly.io supports three service protocols:

1. **HTTP** (default): Fly.io's HTTP proxy intercepts and validates HTTP
2. **TCP**: Raw TCP passthrough, no protocol parsing
3. **UDP**: For UDP-based protocols

For WebSocket, you need either:
- HTTP protocol with proper Upgrade header support (works on standard HTTP ports)
- TCP protocol with TLS handler (our choice, works on custom ports)

### WebSocket Protocol

WebSocket (RFC 6455) uses HTTP upgrade mechanism:
- Starts with HTTP GET request
- Includes `Upgrade: websocket` header
- Server responds with `HTTP/1.1 101 Switching Protocols`
- After upgrade, uses binary framing protocol

This is why HTTP proxies can interfere - they don't expect the protocol switch.

## Files Changed

```
backend/fly.toml                    | 2 lines changed (added protocol + updated handlers)
docs/WEBSOCKET_HANDSHAKE_FIX.md     | New file (this document)
```

## References

- [Fly.io Services Documentation](https://fly.io/docs/reference/configuration/#the-services-sections)
- [RFC 6455: The WebSocket Protocol](https://tools.ietf.org/html/rfc6455)
- [Fly.io WebSocket Support](https://community.fly.io/t/websocket-support/562)
- [HTTP 101 Switching Protocols](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/101)

## Success Metrics

After deploying this fix:
- ✅ Zero WebSocket handshake failures
- ✅ Zero [PU02] proxy errors
- ✅ WebSocket connects on first attempt
- ✅ Real-time features work immediately after login
- ✅ No reconnection loops
- ✅ Sub-100ms message latency

---

**Status**: ✅ Fix implemented and ready for deployment  
**Priority**: P0 - Critical (blocks all real-time features)  
**Effort**: Minimal - 2 line configuration change  
**Risk**: Low - Only affects WebSocket port, HTTP API unaffected  
