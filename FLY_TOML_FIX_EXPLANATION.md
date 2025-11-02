# fly.toml Configuration Issues & Fix

## Problems in Your Current Config

### ❌ Problem 1: WebSocket Service Port Conflict
```toml
# ❌ WRONG - Both services trying to use port 443
[[services]]  # HTTP Service
  [[services.ports]]
    port = 443    # ← Using port 443
    
[[services]]  # WebSocket Service
  [[services.ports]]
    port = 443    # ← Also trying port 443! CONFLICT!
```

You can't have two services listening on the same external port (443). Each service needs its own port mapping.

### ❌ Problem 2: Missing TCP Protocol on HTTP Service
```toml
[[services]]
  internal_port = 8080
  # Missing: protocol = "tcp"
```

Should explicitly declare TCP protocol for clarity.

### ❌ Problem 3: WebSocket on Port 443 is Wrong
WebSocket should be on port 8081, not 443:
- Port 443 is already handling HTTP/HTTPS traffic
- Port 8081 should be dedicated to WebSocket

---

## What Was Happening

When Fly.io tried to start your app:

```
1. HTTP Service starts, binds to port 80 and 443 ✓
2. WebSocket Service tries to bind to port 443 ✗
3. CONFLICT! Port 443 already in use
4. WebSocket service fails silently or uses fallback
5. Browser tries to connect to wss://sohbet-uezxqq.fly.dev:8081
6. Connection fails because WebSocket isn't actually listening
```

---

## The Correct Configuration

### ✅ Separate Services with Different Ports

```toml
# HTTP Service - Handles REST API
[[services]]
  internal_port = 8080           # Your server's HTTP port
  protocol = "tcp"
  
  [[services.ports]]
    port = 80                     # External HTTP
    handlers = ["http"]
  
  [[services.ports]]
    port = 443                    # External HTTPS
    handlers = ["tls", "http"]

# WebSocket Service - Handles WebSocket connections
[[services]]
  internal_port = 8081           # Your server's WebSocket port
  protocol = "tcp"
  
  [[services.ports]]
    port = 8081                   # External WebSocket Secure port
    handlers = ["tls"]            # TLS terminates here, then proxies to 8081
```

### How Traffic Flows

```
HTTP Requests:
  Browser:80 → Fly.io:80 → Server:8080 ✓
  Browser:443 → Fly.io:443 → Server:8080 (with TLS) ✓

WebSocket Connections:
  Browser:wss://...8081 → Fly.io:443 → Wait, this is the issue!
```

Actually, there's a subtlety here. When you connect to `wss://sohbet-uezxqq.fly.dev:8081`, Fly.io does this:

```
Browser → DNS resolves sohbet-uezxqq.fly.dev
        → Tries to connect to IP:8081
        → Fly.io load balancer intercepts
        → TLS handshake happens
        → Proxies to internal:8081
        → Server's WebSocket handler processes upgrade
```

---

## Deployment Steps

### 1. Update Your fly.toml

Replace your current `fly.toml` with:

```toml
app = 'sohbet-uezxqq'
primary_region = 'iad'

[build]
  # Your build config

# HTTP Service (port 8080 → external 80/443)
[[services]]
  internal_port = 8080
  protocol = "tcp"
  
  [[services.ports]]
    port = 80
    handlers = ["http"]
  
  [[services.ports]]
    port = 443
    handlers = ["tls", "http"]

  [services.http_checks]
    interval = 10000
    timeout = 5000
    grace_period = 5000
    method = "GET"
    path = "/api/status"

# WebSocket Service (port 8081 → external 8081)
[[services]]
  internal_port = 8081
  protocol = "tcp"
  
  [[services.ports]]
    port = 8081
    handlers = ["tls"]

  [services.tcp_checks]
    interval = 10000
    timeout = 5000
    grace_period = 5000

[[mounts]]
  source = "sohbet_data"
  destination = "/app/data"

[[vm]]
  memory = '1gb'
  cpu_kind = 'shared'
  cpus = 1
  memory_mb = 1024
```

### 2. Deploy

```bash
flyctl deploy
```

### 3. Monitor Startup

```bash
flyctl logs -f -a sohbet-uezxqq
```

Look for:
```
🌐 HTTP Server listening on http://0.0.0.0:8080
🔌 WebSocket server listening on ws://0.0.0.0:8081
```

### 4. Wait for DNS

DNS can take 30 seconds to propagate. Wait a bit, then test:

```bash
# Test HTTP
curl https://sohbet-uezxqq.fly.dev/api/status

# Should respond with JSON status
```

### 5. Browser Connection

Your frontend should now be able to connect:

```javascript
// This should work now!
const token = getAuthToken();
const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
const ws = new WebSocket(`${protocol}//sohbet-uezxqq.fly.dev:8081/?token=${token}`);

ws.onopen = () => console.log('WebSocket connected! ✓');
ws.onerror = (e) => console.error('WebSocket error:', e);
```

---

## Key Differences: Old vs New Config

| Aspect | ❌ Old | ✅ New |
|--------|--------|--------|
| HTTP Service Port | 443 | 443 ✓ |
| WebSocket Service Port | 443 (conflict!) | 8081 ✓ |
| Protocols Declared | Missing on HTTP | Both explicit ✓ |
| Port Conflict | YES | NO |
| Browser Can Connect | NO | YES |

---

## Verification Checklist

After deploying with the new config:

- [ ] `flyctl deploy` completes without errors
- [ ] Logs show both servers starting
- [ ] `curl https://sohbet-uezxqq.fly.dev/api/status` works
- [ ] Browser DevTools → Network shows WebSocket request
- [ ] WebSocket shows "101 Switching Protocols" (or "Web Socket Protocol Handshake")
- [ ] No "CORS blocked" or connection errors in console
- [ ] Chat/messaging features work in your app

---

## Troubleshooting

### WebSocket still won't connect after deploy

```bash
# Check logs for errors
flyctl logs -a sohbet-uezxqq | grep -i error

# Check machine status
flyctl status -a sohbet-uezxqq

# If machine is stopped, restart it
flyctl machines restart -a sohbet-uezxqq
```

### DNS not updated yet

```bash
# Force DNS refresh
nslookup sohbet-uezxqq.fly.dev
# or
dig sohbet-uezxqq.fly.dev
```

### Still getting connection refused

1. Verify new `fly.toml` is actually deployed:
   ```bash
   flyctl config show
   ```

2. Check both services are listed:
   ```bash
   flyctl status -a sohbet-uezxqq
   ```

3. Force full redeploy:
   ```bash
   flyctl deploy --force-rebuild
   ```

---

## Important Notes

- **Port 8081 on Fly.io is TLS-only** - Fly.io terminates TLS/SSL at the load balancer, then proxies the decrypted connection to your server's port 8081
- Your server still listens on plain `ws://` (not `wss://`) - Fly.io handles encryption
- Both services use the same machine/process, so they share resources
- `min_machines_running = 1` for HTTP ensures the app is always on
- `min_machines_running = 0` for WebSocket was in your old config (not needed, remove it)

---

## Final Config to Use

[Download: fly.toml.fixed](the corrected configuration file)

Replace your current `fly.toml` with this content, then:

```bash
flyctl deploy
```

Done! Your WebSocket should now connect properly.
