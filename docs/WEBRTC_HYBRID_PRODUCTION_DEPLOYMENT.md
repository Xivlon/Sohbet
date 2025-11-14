# WebRTC Hybrid Architecture - Production Deployment Guide

**Document Version:** 2.1 - PRODUCTION DEPLOYMENT ADDENDUM
**Date:** 2025-11-09
**Status:** [WARNING] REQUIRED READING FOR DEPLOYMENT
**Prerequisite:** Read Critical Fixes (v2.0) first

---

## Overview

This addendum addresses critical production deployment gaps:

1. **WebSocket distribution and session affinity**
2. **LiveKit clustering configuration**
3. **Browser-specific compatibility issues**
4. **Private TURN server infrastructure**

**These are NOT optional - they are required for production.**

---

## Table of Contents

1. [WebSocket Distribution & Load Balancing](#websocket-distribution--load-balancing)
2. [LiveKit Clustering Configuration](#livekit-clustering-configuration)
3. [Browser Compatibility Layer](#browser-compatibility-layer)
4. [Private TURN Infrastructure](#private-turn-infrastructure)
5. [Complete Infrastructure Setup](#complete-infrastructure-setup)
6. [Deployment Checklist](#deployment-checklist)

---

## WebSocket Distribution & Load Balancing

### Problem: Multi-Instance Message Routing

**Scenario:**
```
User A connected to Backend Instance 1
User B connected to Backend Instance 2

User A sends message to User B
→ How does Instance 1 route to Instance 2?
```

### Solution: Redis Pub/Sub + Session Affinity

#### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Load Balancer (HAProxy)                  │
│               (Sticky Sessions: source IP hash)             │
└──────┬──────────────────────┬───────────────────┬───────────┘
       │                      │                   │
       │                      │                   │
┌──────▼──────┐      ┌────────▼──────┐   ┌───────▼──────────┐
│ Backend 1   │      │  Backend 2    │   │   Backend N      │
│ (C++ + WS)  │      │  (C++ + WS)   │   │   (C++ + WS)     │
│             │      │               │   │                  │
│ Users:      │      │ Users:        │   │ Users:           │
│ - User A    │      │ - User B      │   │ - User C         │
│ - User D    │      │ - User E      │   │ - User F         │
└──────┬──────┘      └────────┬──────┘   └───────┬──────────┘
       │                      │                   │
       └──────────────────────┼───────────────────┘
                              │
                    ┌─────────▼─────────┐
                    │  Redis Pub/Sub    │
                    │  Channels:        │
                    │  - voice:messages │
                    │  - voice:presence │
                    └───────────────────┘
```

### HAProxy Configuration (Session Affinity)

**File:** `/etc/haproxy/haproxy.cfg`

```haproxy
global
    log /dev/log local0
    maxconn 4096
    ssl-default-bind-ciphers ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384
    ssl-default-bind-options no-sslv3 no-tlsv10 no-tlsv11

defaults
    log global
    mode http
    option httplog
    option dontlognull
    timeout connect 5000ms
    timeout client  50000ms
    timeout server  50000ms

# HTTP frontend (redirects to HTTPS)
frontend http_front
    bind *:80
    redirect scheme https code 301 if !{ ssl_fc }

# HTTPS frontend
frontend https_front
    bind *:443 ssl crt /etc/ssl/certs/sohbet.pem

    # WebSocket detection
    acl is_websocket hdr(Upgrade) -i WebSocket
    acl is_websocket_path path_beg /ws

    # Route WebSocket to backend
    use_backend websocket_backend if is_websocket OR is_websocket_path

    # Route regular HTTP to backend
    default_backend http_backend

# WebSocket backend (sticky sessions)
backend websocket_backend
    balance source  # Source IP hash for sticky sessions
    hash-type consistent

    # Enable WebSocket upgrade
    option http-server-close
    option forwardfor

    # Health check
    option httpchk GET /health
    http-check expect status 200

    # Backend servers
    server backend1 10.0.1.10:8081 check inter 5000 fall 3 rise 2
    server backend2 10.0.1.11:8081 check inter 5000 fall 3 rise 2
    server backend3 10.0.1.12:8081 check inter 5000 fall 3 rise 2

    # Timeouts for long-lived WebSocket connections
    timeout tunnel 3600s
    timeout client 3600s
    timeout server 3600s

# HTTP backend (for REST API)
backend http_backend
    balance roundrobin

    option httpchk GET /health
    http-check expect status 200

    server backend1 10.0.1.10:8080 check
    server backend2 10.0.1.11:8080 check
    server backend3 10.0.1.12:8080 check
```

**Key Configuration:**
- `balance source` - Source IP hashing ensures same client → same backend
- `hash-type consistent` - Minimizes redistribution when backend added/removed
- `timeout tunnel 3600s` - Keeps WebSocket connections alive for 1 hour

### Alternative: Cookie-Based Sticky Sessions

For clients behind NAT (same source IP), use cookies:

```haproxy
backend websocket_backend
    balance roundrobin
    cookie SERVERID insert indirect nocache

    server backend1 10.0.1.10:8081 check cookie backend1
    server backend2 10.0.1.11:8081 check cookie backend2
    server backend3 10.0.1.12:8081 check cookie backend3
```

### Cross-Instance Message Routing (Redis Pub/Sub)

#### Backend Implementation

**File:** `src/server/websocket_cluster.cpp`

```cpp
#include <sw/redis++/redis.h>
#include <thread>
#include <nlohmann/json.hpp>

using namespace sw::redis;
using json = nlohmann::json;

class WebSocketCluster {
private:
    Redis redis;
    Subscriber subscriber;
    std::string instance_id;

    // Local WebSocket connections (user_id → connection)
    std::map<int, WebSocketConnection*> local_connections;
    std::mutex connections_mutex;

public:
    WebSocketCluster(const std::string& redis_url)
        : redis(redis_url),
          subscriber(redis.subscriber()),
          instance_id(generateInstanceId()) {

        // Subscribe to broadcast channel
        subscriber.on_message([this](std::string channel, std::string msg) {
            this->handleClusterMessage(msg);
        });

        subscriber.subscribe("voice:cluster");

        // Start subscription thread
        std::thread([this]() {
            while (true) {
                try {
                    subscriber.consume();
                } catch (const Error& e) {
                    std::cerr << "Redis subscription error: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }).detach();

        std::cout << "WebSocket cluster initialized (instance: " << instance_id << ")" << std::endl;
    }

    /**
     * Register local WebSocket connection
     */
    void registerConnection(int user_id, WebSocketConnection* conn) {
        std::lock_guard<std::mutex> lock(connections_mutex);
        local_connections[user_id] = conn;

        // Publish presence to cluster
        publishPresence(user_id, "connected");

        // Store mapping in Redis for lookup
        redis.hset("ws_user_instances", std::to_string(user_id), instance_id);
    }

    /**
     * Unregister local WebSocket connection
     */
    void unregisterConnection(int user_id) {
        std::lock_guard<std::mutex> lock(connections_mutex);
        local_connections.erase(user_id);

        // Publish presence to cluster
        publishPresence(user_id, "disconnected");

        // Remove mapping from Redis
        redis.hdel("ws_user_instances", std::to_string(user_id));
    }

    /**
     * Send message to user (handles cross-instance routing)
     */
    void sendToUser(int user_id, const json& message) {
        // Check if user is connected locally
        {
            std::lock_guard<std::mutex> lock(connections_mutex);
            auto it = local_connections.find(user_id);
            if (it != local_connections.end()) {
                // Local delivery
                it->second->send(message.dump());
                return;
            }
        }

        // User not local - find their instance via Redis
        auto instance_opt = redis.hget("ws_user_instances", std::to_string(user_id));

        if (!instance_opt) {
            std::cerr << "User " << user_id << " not connected to any instance" << std::endl;
            return;
        }

        std::string target_instance = *instance_opt;

        // Publish to cluster for routing
        json cluster_msg = {
            {"type", "route_to_user"},
            {"target_instance", target_instance},
            {"user_id", user_id},
            {"payload", message}
        };

        redis.publish("voice:cluster", cluster_msg.dump());
    }

    /**
     * Broadcast to all users in channel
     */
    void broadcastToChannel(int channel_id, const json& message) {
        // Get all users in channel from Redis
        auto user_ids = redis.smembers("channel_participants:" + std::to_string(channel_id));

        for (const auto& user_id_str : user_ids) {
            int user_id = std::stoi(user_id_str);
            sendToUser(user_id, message);
        }
    }

private:
    /**
     * Handle messages from other instances
     */
    void handleClusterMessage(const std::string& msg) {
        try {
            json cluster_msg = json::parse(msg);
            std::string type = cluster_msg["type"];

            if (type == "route_to_user") {
                std::string target_instance = cluster_msg["target_instance"];

                // Is this message for us?
                if (target_instance != instance_id) {
                    return; // Not for us
                }

                int user_id = cluster_msg["user_id"];
                json payload = cluster_msg["payload"];

                // Deliver to local connection
                std::lock_guard<std::mutex> lock(connections_mutex);
                auto it = local_connections.find(user_id);
                if (it != local_connections.end()) {
                    it->second->send(payload.dump());
                }

            } else if (type == "presence") {
                // Handle presence updates (user connected/disconnected)
                handlePresenceUpdate(cluster_msg);
            }

        } catch (const std::exception& e) {
            std::cerr << "Error handling cluster message: " << e.what() << std::endl;
        }
    }

    /**
     * Publish presence update
     */
    void publishPresence(int user_id, const std::string& status) {
        json presence = {
            {"type", "presence"},
            {"instance_id", instance_id},
            {"user_id", user_id},
            {"status", status},
            {"timestamp", getCurrentTimestamp()}
        };

        redis.publish("voice:cluster", presence.dump());
    }

    /**
     * Handle presence updates from other instances
     */
    void handlePresenceUpdate(const json& presence) {
        int user_id = presence["user_id"];
        std::string status = presence["status"];

        std::cout << "User " << user_id << " " << status
                  << " on instance " << presence["instance_id"] << std::endl;
    }

    /**
     * Generate unique instance ID
     */
    std::string generateInstanceId() {
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        return std::string(hostname) + "-" + std::to_string(getpid());
    }

    int64_t getCurrentTimestamp() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
};
```

### Connection State Tracking

**Redis Schema:**

```
# User → Instance mapping
ws_user_instances -> Hash{
  "100": "backend1-12345",
  "101": "backend2-67890",
  "102": "backend1-12345"
}

# Instance → Users mapping (for cleanup on instance crash)
ws_instance_users:backend1-12345 -> Set{100, 102}
ws_instance_users:backend2-67890 -> Set{101}

# Heartbeat (detect dead instances)
ws_instance_heartbeat:backend1-12345 -> timestamp (expires in 30s)
```

### Instance Health Monitoring

```cpp
class InstanceHealthMonitor {
private:
    Redis redis;
    std::string instance_id;

public:
    /**
     * Send heartbeat every 10 seconds
     */
    void startHeartbeat() {
        std::thread([this]() {
            while (true) {
                try {
                    std::string key = "ws_instance_heartbeat:" + instance_id;
                    redis.set(key, std::to_string(getCurrentTimestamp()));
                    redis.expire(key, std::chrono::seconds(30));

                    std::this_thread::sleep_for(std::chrono::seconds(10));
                } catch (const Error& e) {
                    std::cerr << "Heartbeat error: " << e.what() << std::endl;
                }
            }
        }).detach();
    }

    /**
     * Detect and clean up dead instances
     */
    void detectDeadInstances() {
        auto instance_keys = redis.keys("ws_instance_users:*");

        for (const auto& key : instance_keys) {
            // Extract instance ID
            std::string instance = key.substr(strlen("ws_instance_users:"));

            // Check heartbeat
            std::string hb_key = "ws_instance_heartbeat:" + instance;
            auto heartbeat = redis.get(hb_key);

            if (!heartbeat) {
                // Instance is dead - clean up
                std::cout << "Detected dead instance: " << instance << std::endl;
                cleanupDeadInstance(instance);
            }
        }
    }

    /**
     * Clean up connections from dead instance
     */
    void cleanupDeadInstance(const std::string& instance) {
        // Get all users from dead instance
        std::string users_key = "ws_instance_users:" + instance;
        auto user_ids = redis.smembers(users_key);

        for (const auto& user_id_str : user_ids) {
            // Remove from user → instance mapping
            redis.hdel("ws_user_instances", user_id_str);

            std::cout << "Cleaned up user " << user_id_str
                      << " from dead instance " << instance << std::endl;
        }

        // Delete instance users set
        redis.del(users_key);

        // Notify channels of disconnected users
        // (trigger voice:user-left events)
    }
};
```

---

## LiveKit Clustering Configuration

### Problem: Single LiveKit Instance = SPOF

**Issues:**
- Single point of failure
- Limited capacity (50-100 users per instance)
- No geographic distribution

### Solution: LiveKit Cluster with Redis Coordination

#### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                  Backend (Room Assignment)                   │
│            Consistent Hashing: channel_id → node            │
└──────────────────────┬──────────────────────────────────────┘
                       │
       ┌───────────────┼───────────────┐
       │               │               │
┌──────▼──────┐ ┌──────▼──────┐ ┌─────▼───────┐
│ LiveKit     │ │ LiveKit     │ │ LiveKit     │
│ Node 1      │ │ Node 2      │ │ Node N      │
│             │ │             │ │             │
│ Rooms:      │ │ Rooms:      │ │ Rooms:      │
│ - channel_1 │ │ - channel_2 │ │ - channel_3 │
│ - channel_4 │ │ - channel_5 │ │ - channel_6 │
└──────┬──────┘ └──────┬──────┘ └─────┬───────┘
       │               │               │
       └───────────────┼───────────────┘
                       │
              ┌────────▼────────┐
              │  Redis Cluster  │
              │  - Room routing │
              │  - Node health  │
              └─────────────────┘
```

### LiveKit Configuration (Clustered)

**File:** `livekit.yaml` (for each node)

```yaml
port: 7880
bind_addresses:
  - 0.0.0.0

# API Keys
keys:
  ${LIVEKIT_API_KEY}: ${LIVEKIT_API_SECRET}

# Redis for clustering
redis:
  address: redis-cluster:6379
  db: 0
  use_tls: false

# Room settings
room:
  auto_create: true
  empty_timeout: 300  # 5 minutes
  max_participants: 100

# RTC configuration
rtc:
  # Use different port ranges per node to avoid conflicts
  port_range_start: ${PORT_RANGE_START}  # Node 1: 50000, Node 2: 60000, etc.
  port_range_end: ${PORT_RANGE_END}      # Node 1: 59999, Node 2: 69999, etc.
  use_external_ip: true

  # TURN server (covered in next section)
  turn_servers:
    - host: turn.sohbet.com
      port: 3478
      username: ${TURN_USERNAME}
      credential: ${TURN_PASSWORD}
      protocol: udp

# Node configuration
node:
  ip: ${NODE_IP}  # External IP of this node

# Logging
logging:
  level: info

# Webhooks (for room events)
webhook:
  api_key: ${WEBHOOK_API_KEY}
  urls:
    - http://backend:8080/webhooks/livekit
```

### Docker Compose (Multi-Node LiveKit)

**File:** `docker-compose-livekit-cluster.yml`

```yaml
version: '3.8'

services:
  # Redis for LiveKit coordination
  redis-livekit:
    image: redis:7-alpine
    ports:
      - "6379:6379"
    volumes:
      - redis-livekit-data:/data
    command: redis-server --appendonly yes

  # LiveKit Node 1
  livekit-node-1:
    image: livekit/livekit-server:latest
    ports:
      - "7880:7880"    # API
      - "7881:7881"    # HTTP
      - "50000-59999:50000-59999/udp"  # RTC
    environment:
      - LIVEKIT_API_KEY=${LIVEKIT_API_KEY}
      - LIVEKIT_API_SECRET=${LIVEKIT_API_SECRET}
      - PORT_RANGE_START=50000
      - PORT_RANGE_END=59999
      - NODE_IP=${LIVEKIT_NODE_1_IP}
      - TURN_USERNAME=${TURN_USERNAME}
      - TURN_PASSWORD=${TURN_PASSWORD}
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    command: --config /etc/livekit.yaml
    depends_on:
      - redis-livekit
    restart: unless-stopped

  # LiveKit Node 2
  livekit-node-2:
    image: livekit/livekit-server:latest
    ports:
      - "7882:7880"    # API (different port)
      - "7883:7881"    # HTTP
      - "60000-69999:60000-69999/udp"  # RTC (different range)
    environment:
      - LIVEKIT_API_KEY=${LIVEKIT_API_KEY}
      - LIVEKIT_API_SECRET=${LIVEKIT_API_SECRET}
      - PORT_RANGE_START=60000
      - PORT_RANGE_END=69999
      - NODE_IP=${LIVEKIT_NODE_2_IP}
      - TURN_USERNAME=${TURN_USERNAME}
      - TURN_PASSWORD=${TURN_PASSWORD}
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    command: --config /etc/livekit.yaml
    depends_on:
      - redis-livekit
    restart: unless-stopped

  # LiveKit Node 3
  livekit-node-3:
    image: livekit/livekit-server:latest
    ports:
      - "7884:7880"
      - "7885:7881"
      - "70000-79999:70000-79999/udp"
    environment:
      - LIVEKIT_API_KEY=${LIVEKIT_API_KEY}
      - LIVEKIT_API_SECRET=${LIVEKIT_API_SECRET}
      - PORT_RANGE_START=70000
      - PORT_RANGE_END=79999
      - NODE_IP=${LIVEKIT_NODE_3_IP}
      - TURN_USERNAME=${TURN_USERNAME}
      - TURN_PASSWORD=${TURN_PASSWORD}
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    command: --config /etc/livekit.yaml
    depends_on:
      - redis-livekit
    restart: unless-stopped

volumes:
  redis-livekit-data:
```

### Room Assignment Strategy (Backend)

**Consistent Hashing for Room Distribution:**

```cpp
#include <functional>
#include <map>

class LiveKitCluster {
private:
    struct LiveKitNode {
        std::string url;
        std::string api_key;
        std::string api_secret;
        int capacity;
        int current_rooms;
    };

    std::vector<LiveKitNode> nodes;
    Redis redis;

public:
    LiveKitCluster(Redis& r) : redis(r) {
        // Load nodes from config
        nodes = {
            {"ws://livekit-node-1:7880", getenv("LIVEKIT_API_KEY"), getenv("LIVEKIT_API_SECRET"), 50, 0},
            {"ws://livekit-node-2:7882", getenv("LIVEKIT_API_KEY"), getenv("LIVEKIT_API_SECRET"), 50, 0},
            {"ws://livekit-node-3:7884", getenv("LIVEKIT_API_KEY"), getenv("LIVEKIT_API_SECRET"), 50, 0}
        };
    }

    /**
     * Assign channel to LiveKit node using consistent hashing
     */
    LiveKitNode& assignNode(int channel_id) {
        // Check if channel already assigned
        std::string mapping_key = "livekit_room_node:" + std::to_string(channel_id);
        auto existing_node = redis.get(mapping_key);

        if (existing_node) {
            // Return existing assignment
            int node_index = std::stoi(*existing_node);
            return nodes[node_index];
        }

        // Use consistent hashing to assign
        std::hash<int> hasher;
        size_t hash = hasher(channel_id);
        size_t node_index = hash % nodes.size();

        // Check if node has capacity
        if (nodes[node_index].current_rooms >= nodes[node_index].capacity) {
            // Find node with capacity
            for (size_t i = 0; i < nodes.size(); i++) {
                if (nodes[i].current_rooms < nodes[i].capacity) {
                    node_index = i;
                    break;
                }
            }
        }

        // Store assignment in Redis
        redis.set(mapping_key, std::to_string(node_index));
        redis.expire(mapping_key, std::chrono::hours(24));

        nodes[node_index].current_rooms++;

        return nodes[node_index];
    }

    /**
     * Generate token for specific node
     */
    std::string generateTokenForChannel(int channel_id, int user_id, const std::string& username) {
        auto& node = assignNode(channel_id);

        // Generate JWT for this specific node
        auto token = jwt::create()
            .set_issuer(node.api_key)
            .set_subject(std::to_string(user_id))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes(5))
            .set_payload_claim("name", jwt::claim(username))
            .set_payload_claim("room", jwt::claim("channel_" + std::to_string(channel_id)))
            .set_payload_claim("video", jwt::claim(picojson::value(false)))
            .set_payload_claim("audio", jwt::claim(picojson::value(true)))
            .sign(jwt::algorithm::hs256{node.api_secret});

        return token;
    }

    /**
     * Get WebSocket URL for channel
     */
    std::string getNodeUrl(int channel_id) {
        auto& node = assignNode(channel_id);
        return node.url;
    }
};
```

### Node Health Monitoring

```cpp
class LiveKitHealthMonitor {
private:
    std::vector<std::string> node_urls;
    Redis redis;

public:
    /**
     * Check health of all LiveKit nodes
     */
    void monitorHealth() {
        for (size_t i = 0; i < node_urls.size(); i++) {
            std::string health_url = node_urls[i] + "/health";

            // HTTP health check
            auto response = http_client.get(health_url);

            bool is_healthy = (response.status == 200);

            // Store health status in Redis
            std::string health_key = "livekit_node_health:" + std::to_string(i);
            redis.set(health_key, is_healthy ? "1" : "0");
            redis.expire(health_key, std::chrono::seconds(30));

            if (!is_healthy) {
                std::cerr << "LiveKit node " << i << " is unhealthy!" << std::endl;

                // Alert operations
                sendAlert("LiveKit Node Down", "Node " + std::to_string(i) + " is unhealthy");

                // Migrate rooms to healthy nodes
                migrateRoomsFromNode(i);
            }
        }
    }

    /**
     * Migrate rooms from failed node
     */
    void migrateRoomsFromNode(int failed_node_index) {
        // Get all rooms assigned to failed node
        auto room_keys = redis.keys("livekit_room_node:*");

        for (const auto& key : room_keys) {
            auto node_index = redis.get(key);

            if (node_index && std::stoi(*node_index) == failed_node_index) {
                // Reassign to different node
                std::string channel_id_str = key.substr(strlen("livekit_room_node:"));
                int channel_id = std::stoi(channel_id_str);

                // Delete old assignment
                redis.del(key);

                // Notify users to reconnect (triggers new assignment)
                notifyRoomReconnect(channel_id);
            }
        }
    }
};
```

---

## Browser Compatibility Layer

### Problem: Safari and Mobile Browser Differences

**Key Issues:**
1. **Safari AudioContext** requires user gesture
2. **Codec support** varies (Opus vs AAC)
3. **Mobile resource limits** (battery, CPU)
4. **iOS Safari** specific bugs

### Solution: Browser Detection + Compatibility Shims

#### Browser Detection

```typescript
class BrowserDetector {
  static isSafari(): boolean {
    return /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
  }

  static isIOS(): boolean {
    return /iPad|iPhone|iPod/.test(navigator.userAgent);
  }

  static isChrome(): boolean {
    return /Chrome/.test(navigator.userAgent) && /Google Inc/.test(navigator.vendor);
  }

  static isFirefox(): boolean {
    return /Firefox/.test(navigator.userAgent);
  }

  static isMobile(): boolean {
    return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
  }

  static getCapabilities(): BrowserCapabilities {
    return {
      isSafari: this.isSafari(),
      isIOS: this.isIOS(),
      isMobile: this.isMobile(),
      supportsOpus: this.checkOpusSupport(),
      supportsH264: this.checkH264Support(),
      audioContextRequiresGesture: this.isSafari() || this.isIOS(),
      maxParticipantsRecommended: this.isMobile() ? 10 : 50
    };
  }

  private static checkOpusSupport(): boolean {
    const pc = new RTCPeerConnection();
    const supported = pc.createOffer().then(offer => {
      return offer.sdp?.includes('opus') ?? false;
    });
    pc.close();
    return supported;
  }

  private static checkH264Support(): boolean {
    const pc = new RTCPeerConnection();
    const supported = pc.createOffer().then(offer => {
      return offer.sdp?.includes('H264') ?? false;
    });
    pc.close();
    return supported;
  }
}
```

#### Safari AudioContext Fix

**Problem:** Safari requires user gesture to start AudioContext

```typescript
class SafariAudioContextFix {
  private audioContext: AudioContext | null = null;
  private isUnlocked: boolean = false;

  /**
   * Create AudioContext with user gesture requirement
   */
  async getAudioContext(): Promise<AudioContext> {
    if (this.audioContext && this.isUnlocked) {
      return this.audioContext;
    }

    // Create AudioContext
    this.audioContext = new (window.AudioContext || (window as any).webkitAudioContext)();

    // Safari requires resume() after user gesture
    if (BrowserDetector.isSafari() || BrowserDetector.isIOS()) {
      if (this.audioContext.state === 'suspended') {
        // Wait for user gesture
        await this.waitForUserGesture();
      }
    }

    return this.audioContext;
  }

  /**
   * Wait for user interaction before resuming AudioContext
   */
  private async waitForUserGesture(): Promise<void> {
    return new Promise((resolve) => {
      const events = ['touchstart', 'touchend', 'mousedown', 'keydown'];

      const unlockAudio = async () => {
        if (!this.audioContext) return;

        try {
          await this.audioContext.resume();
          this.isUnlocked = true;

          // Remove event listeners
          events.forEach(event => {
            document.removeEventListener(event, unlockAudio);
          });

          console.log('[Safari] AudioContext unlocked after user gesture');
          resolve();
        } catch (error) {
          console.error('[Safari] Failed to unlock AudioContext:', error);
        }
      };

      // Listen for any user interaction
      events.forEach(event => {
        document.addEventListener(event, unlockAudio, { once: true });
      });

      // Also try immediate resume (might work if already interacted)
      unlockAudio();
    });
  }

  /**
   * Show UI prompt for user gesture
   */
  showGesturePrompt(): void {
    const prompt = document.createElement('div');
    prompt.id = 'audio-unlock-prompt';
    prompt.innerHTML = `
      <div style="
        position: fixed;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        background: white;
        padding: 20px;
        border-radius: 8px;
        box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        z-index: 10000;
        text-align: center;
      ">
        <h3>Enable Audio</h3>
        <p>Tap to enable voice chat</p>
        <button id="enable-audio-btn" style="
          padding: 10px 20px;
          background: #007bff;
          color: white;
          border: none;
          border-radius: 4px;
          cursor: pointer;
        ">Enable Audio</button>
      </div>
    `;

    document.body.appendChild(prompt);

    document.getElementById('enable-audio-btn')?.addEventListener('click', async () => {
      if (this.audioContext) {
        await this.audioContext.resume();
        this.isUnlocked = true;
        prompt.remove();
      }
    });
  }
}
```

#### Codec Negotiation

```typescript
class CodecNegotiator {
  /**
   * Prefer Opus, fallback to other codecs for compatibility
   */
  static async setupPreferredCodecs(
    peerConnection: RTCPeerConnection,
    offer: RTCSessionDescriptionInit
  ): Promise<RTCSessionDescriptionInit> {
    const capabilities = BrowserDetector.getCapabilities();

    // Modify SDP to prefer codecs
    let sdp = offer.sdp || '';

    if (capabilities.supportsOpus) {
      // Prefer Opus for audio (best quality)
      sdp = this.preferCodec(sdp, 'audio', 'opus');
    } else {
      // Fallback to PCMU/PCMA for Safari
      sdp = this.preferCodec(sdp, 'audio', 'PCMU');
    }

    return {
      type: offer.type,
      sdp: sdp
    };
  }

  /**
   * Modify SDP to prefer specific codec
   */
  private static preferCodec(sdp: string, type: string, codec: string): string {
    const sdpLines = sdp.split('\r\n');
    const mLineIndex = sdpLines.findIndex(line => line.startsWith(`m=${type}`));

    if (mLineIndex === -1) return sdp;

    // Find codec payload type
    const codecRegex = new RegExp(`a=rtpmap:(\\d+) ${codec}/`, 'i');
    let codecPayloadType: string | null = null;

    for (let i = mLineIndex; i < sdpLines.length; i++) {
      const match = sdpLines[i].match(codecRegex);
      if (match) {
        codecPayloadType = match[1];
        break;
      }
    }

    if (!codecPayloadType) return sdp;

    // Reorder m= line to prefer this codec
    const mLineParts = sdpLines[mLineIndex].split(' ');
    const payloadTypes = mLineParts.slice(3); // Skip "m=audio PORT PROTO"

    // Move preferred codec to front
    const index = payloadTypes.indexOf(codecPayloadType);
    if (index > 0) {
      payloadTypes.splice(index, 1);
      payloadTypes.unshift(codecPayloadType);
    }

    sdpLines[mLineIndex] = mLineParts.slice(0, 3).concat(payloadTypes).join(' ');

    return sdpLines.join('\r\n');
  }
}
```

#### Mobile Resource Management

```typescript
class MobileResourceManager {
  /**
   * Detect and handle low battery
   */
  static async monitorBattery(): Promise<void> {
    if (!('getBattery' in navigator)) {
      return; // Battery API not supported
    }

    const battery = await (navigator as any).getBattery();

    battery.addEventListener('levelchange', () => {
      if (battery.level < 0.15) { // Below 15%
        console.warn('[Mobile] Low battery detected, reducing quality');
        this.enableLowPowerMode();
      }
    });

    battery.addEventListener('chargingchange', () => {
      if (battery.charging) {
        this.disableLowPowerMode();
      }
    });
  }

  /**
   * Enable low power mode (reduce quality)
   */
  private static enableLowPowerMode(): void {
    // Reduce audio bitrate
    webrtcService.setAudioBitrate(16000); // 16 kbps instead of 64 kbps

    // Disable video if enabled
    webrtcService.disableVideo();

    // Show notification to user
    alert('Low battery detected. Voice quality reduced to save power.');
  }

  /**
   * Disable low power mode
   */
  private static disableLowPowerMode(): void {
    webrtcService.setAudioBitrate(64000); // Restore to 64 kbps
  }

  /**
   * Detect thermal throttling (iOS)
   */
  static monitorPerformance(): void {
    if (!BrowserDetector.isIOS()) return;

    // Monitor frame rate drops as proxy for thermal throttling
    let lastTime = performance.now();
    let frameCount = 0;

    const checkFrameRate = () => {
      frameCount++;
      const now = performance.now();

      if (now - lastTime >= 1000) { // Every second
        const fps = frameCount;
        frameCount = 0;
        lastTime = now;

        if (fps < 30) { // Throttling detected
          console.warn('[iOS] Performance degradation detected, reducing load');
          this.reduceLoad();
        }
      }

      requestAnimationFrame(checkFrameRate);
    };

    requestAnimationFrame(checkFrameRate);
  }

  private static reduceLoad(): void {
    // Reduce number of simultaneous connections
    const maxConnections = 5;
    webrtcService.setMaxConnections(maxConnections);
  }
}
```

#### iOS Safari Specific Fixes

```typescript
class IOSSafariFixes {
  /**
   * Fix for iOS Safari audio interruption on phone call
   */
  static setupAudioInterruptionHandling(): void {
    if (!BrowserDetector.isIOS()) return;

    document.addEventListener('visibilitychange', async () => {
      if (document.hidden) {
        // App went to background
        console.log('[iOS] App backgrounded, pausing audio');
        webrtcService.pauseAllAudio();
      } else {
        // App came to foreground
        console.log('[iOS] App foregrounded, resuming audio');
        await webrtcService.resumeAllAudio();
      }
    });
  }

  /**
   * Fix for iOS Safari WebRTC video freezing
   */
  static setupVideoFreezeDetection(): void {
    if (!BrowserDetector.isIOS()) return;

    // Monitor video elements for freezing
    setInterval(() => {
      const videos = document.querySelectorAll('video');
      videos.forEach(video => {
        if (video.paused && video.readyState >= 2) {
          // Video should be playing but is paused
          console.warn('[iOS] Detected frozen video, restarting');
          video.play().catch(e => console.error('Failed to restart video:', e));
        }
      });
    }, 5000);
  }

  /**
   * Fix for iOS Safari getUserMedia requiring HTTPS
   */
  static checkSecureContext(): boolean {
    if (!window.isSecureContext) {
      alert('Voice chat requires HTTPS. Please access via https://');
      return false;
    }
    return true;
  }
}
```

---

## Private TURN Infrastructure

### Problem: Public TURN Servers Are NOT Production-Ready

**Issues with openrelay.metered.ca:**
- Rate limited (unknown limits)
- Shared with everyone (performance unpredictable)
- No SLA or uptime guarantee
- Geographic latency varies wildly

### Solution: Self-Hosted coturn Servers

#### Architecture (Geographic Distribution)

```
┌──────────────────────────────────────────────────────────┐
│                   Global TURN Infrastructure             │
│                                                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │  US East    │  │  EU West    │  │  Asia Pac   │     │
│  │  TURN       │  │  TURN       │  │  TURN       │     │
│  │  (Virginia) │  │  (Ireland)  │  │  (Tokyo)    │     │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘     │
│         │                │                │             │
│         └────────────────┼────────────────┘             │
│                          │                               │
│              ┌───────────▼───────────┐                  │
│              │   GeoIP Router        │                  │
│              │   (HAProxy)           │                  │
│              └───────────────────────┘                  │
└──────────────────────────────────────────────────────────┘
```

#### coturn Installation (Ubuntu)

```bash
# Install coturn
sudo apt update
sudo apt install coturn

# Enable coturn service
sudo systemctl enable coturn
```

#### coturn Configuration

**File:** `/etc/turnserver.conf`

```conf
# TURN server configuration

# Listening ports
listening-port=3478
tls-listening-port=5349

# Relay ports (UDP)
min-port=49152
max-port=65535

# External IP (public IP of server)
external-ip=YOUR_PUBLIC_IP/YOUR_PRIVATE_IP

# Realm (your domain)
realm=turn.sohbet.com
server-name=turn.sohbet.com

# Authentication
lt-cred-mech
use-auth-secret
static-auth-secret=YOUR_TURN_SECRET_KEY

# SSL certificates (for TLS)
cert=/etc/letsencrypt/live/turn.sohbet.com/fullchain.pem
pkey=/etc/letsencrypt/live/turn.sohbet.com/privkey.pem

# Logging
log-file=/var/log/turnserver.log
verbose

# Performance tuning
user-quota=100
total-quota=1200

# Deny loopback and private IPs as relay addresses
no-loopback-peers
no-multicast-peers

# Limit relay bandwidth per user (in KB/s)
max-bps=1000000  # 1 Mbps per user

# Enable Prometheus metrics
prometheus
```

#### SSL Certificate (Let's Encrypt)

```bash
# Install certbot
sudo apt install certbot

# Get certificate
sudo certbot certonly --standalone -d turn.sohbet.com

# Auto-renewal
sudo crontab -e
# Add: 0 0 * * * certbot renew --quiet && systemctl restart coturn
```

#### Start coturn

```bash
sudo systemctl start coturn
sudo systemctl status coturn
```

#### TURN Server Monitoring

```bash
# Check coturn status
sudo turnutils_uclient -v -u test -w password turn.sohbet.com

# Monitor active allocations
sudo turnserver --log-file=stdout | grep "allocation"

# Check Prometheus metrics
curl http://localhost:9641/metrics
```

#### Geographic Router (HAProxy)

**Route users to nearest TURN server based on GeoIP:**

```haproxy
# /etc/haproxy/haproxy-turn.cfg

frontend turn_front
    bind *:3478
    mode tcp

    # GeoIP detection
    acl is_us_user src_geoip_country(US,CA,MX)
    acl is_eu_user src_geoip_country(GB,DE,FR,IT,ES,NL,PL)
    acl is_asia_user src_geoip_country(JP,KR,CN,SG,IN)

    # Route to nearest server
    use_backend turn_us if is_us_user
    use_backend turn_eu if is_eu_user
    use_backend turn_asia if is_asia_user

    # Default to US
    default_backend turn_us

backend turn_us
    mode tcp
    server turn-us-1 turn-us.sohbet.com:3478 check

backend turn_eu
    mode tcp
    server turn-eu-1 turn-eu.sohbet.com:3478 check

backend turn_asia
    mode tcp
    server turn-asia-1 turn-asia.sohbet.com:3478 check
```

#### TURN Server Credentials (Time-Limited)

```cpp
/**
 * Generate time-limited TURN credentials
 */
class TURNCredentialGenerator {
public:
    struct TURNCredentials {
        std::string username;
        std::string password;
        std::vector<std::string> urls;
    };

    static TURNCredentials generate(const std::string& user_id) {
        const std::string turn_secret = getenv("TURN_SECRET_KEY");
        const int64_t ttl = 24 * 3600; // 24 hours

        // Unix timestamp + TTL
        int64_t expiry = getCurrentTimestamp() / 1000 + ttl;

        // Username format: timestamp:user_id
        std::string username = std::to_string(expiry) + ":" + user_id;

        // Password = base64(hmac_sha1(secret, username))
        std::string password = generateHMAC_SHA1(turn_secret, username);

        return {
            .username = username,
            .password = password,
            .urls = {
                "turn:turn.sohbet.com:3478?transport=udp",
                "turn:turn.sohbet.com:3478?transport=tcp",
                "turns:turn.sohbet.com:5349?transport=tcp"  // TLS
            }
        };
    }

private:
    static std::string generateHMAC_SHA1(const std::string& key, const std::string& data) {
        // Use OpenSSL HMAC
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;

        HMAC(EVP_sha1(),
             key.c_str(), key.length(),
             (unsigned char*)data.c_str(), data.length(),
             hash, &hash_len);

        // Base64 encode
        return base64_encode(hash, hash_len);
    }
};
```

#### Frontend: Use Private TURN

```typescript
const DEFAULT_ICE_SERVERS: RTCIceServer[] = [
  // STUN (free, low bandwidth)
  { urls: 'stun:stun.l.google.com:19302' },

  // Private TURN (requires credentials)
  {
    urls: turnCredentials.urls,  // From backend
    username: turnCredentials.username,
    credential: turnCredentials.password,
  }
];
```

#### TURN Server Cost Estimation

**Per Server:**
- Instance: 4 vCPU, 8GB RAM = $50/month
- Bandwidth: 5TB @ $0.09/GB = $450/month
- **Total per region: $500/month**

**Three Regions (US, EU, Asia):**
- **Total: $1,500/month**

**Cost per User (1 hour/day, 50% TURN usage):**
- Bandwidth: 64 Kbps × 3600s × 30 days × 0.5 = ~3.5 GB/month
- Cost: 3.5 GB × $0.09 = **$0.32/user/month**

---

## Complete Infrastructure Setup

### Full Production Stack

**Services Required:**

| Component | Instances | Monthly Cost | Purpose |
|-----------|-----------|--------------|---------|
| **Backend (C++)** | 3 | $150 | WebSocket, REST API |
| **Redis Cluster** | 3 (1 master + 2 replicas) | $90 | State store |
| **LiveKit SFU** | 3 | $450 | Media routing |
| **coturn TURN** | 3 (US, EU, Asia) | $1,500 | NAT traversal |
| **HAProxy LB** | 2 (active-passive) | $60 | Load balancing |
| **Monitoring** | 1 | $50 | Prometheus + Grafana |
| **Total** | 15 | **$2,300/month** | Full production |

### docker-compose.yml (Complete)

**File:** `docker-compose-production.yml`

```yaml
version: '3.8'

services:
  # ===== Redis Cluster =====
  redis-master:
    image: redis:7-alpine
    command: redis-server --appendonly yes
    volumes:
      - redis-master-data:/data
    networks:
      - backend-net

  redis-replica-1:
    image: redis:7-alpine
    command: redis-server --replicaof redis-master 6379 --appendonly yes
    volumes:
      - redis-replica1-data:/data
    depends_on:
      - redis-master
    networks:
      - backend-net

  redis-replica-2:
    image: redis:7-alpine
    command: redis-server --replicaof redis-master 6379 --appendonly yes
    volumes:
      - redis-replica2-data:/data
    depends_on:
      - redis-master
    networks:
      - backend-net

  redis-sentinel-1:
    image: redis:7-alpine
    command: redis-sentinel /etc/redis/sentinel.conf
    volumes:
      - ./redis-sentinel.conf:/etc/redis/sentinel.conf
    depends_on:
      - redis-master
    networks:
      - backend-net

  # ===== Backend Instances =====
  backend-1:
    build: ./backend
    environment:
      - REDIS_URL=redis://redis-master:6379
      - INSTANCE_ID=backend-1
      - LIVEKIT_NODES=livekit-1:7880,livekit-2:7882,livekit-3:7884
    ports:
      - "8080:8080"
      - "8081:8081"
    depends_on:
      - redis-master
    networks:
      - backend-net

  backend-2:
    build: ./backend
    environment:
      - REDIS_URL=redis://redis-master:6379
      - INSTANCE_ID=backend-2
      - LIVEKIT_NODES=livekit-1:7880,livekit-2:7882,livekit-3:7884
    ports:
      - "8082:8080"
      - "8083:8081"
    depends_on:
      - redis-master
    networks:
      - backend-net

  backend-3:
    build: ./backend
    environment:
      - REDIS_URL=redis://redis-master:6379
      - INSTANCE_ID=backend-3
      - LIVEKIT_NODES=livekit-1:7880,livekit-2:7882,livekit-3:7884
    ports:
      - "8084:8080"
      - "8085:8081"
    depends_on:
      - redis-master
    networks:
      - backend-net

  # ===== LiveKit Cluster =====
  livekit-1:
    image: livekit/livekit-server:latest
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    ports:
      - "7880:7880"
      - "50000-59999:50000-59999/udp"
    environment:
      - NODE_IP=${LIVEKIT_NODE_1_IP}
      - PORT_RANGE_START=50000
      - PORT_RANGE_END=59999
    networks:
      - backend-net

  livekit-2:
    image: livekit/livekit-server:latest
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    ports:
      - "7882:7880"
      - "60000-69999:60000-69999/udp"
    environment:
      - NODE_IP=${LIVEKIT_NODE_2_IP}
      - PORT_RANGE_START=60000
      - PORT_RANGE_END=69999
    networks:
      - backend-net

  livekit-3:
    image: livekit/livekit-server:latest
    volumes:
      - ./livekit.yaml:/etc/livekit.yaml
    ports:
      - "7884:7880"
      - "70000-79999:70000-79999/udp"
    environment:
      - NODE_IP=${LIVEKIT_NODE_3_IP}
      - PORT_RANGE_START=70000
      - PORT_RANGE_END=79999
    networks:
      - backend-net

  # ===== HAProxy Load Balancer =====
  haproxy:
    image: haproxy:latest
    volumes:
      - ./haproxy.cfg:/usr/local/etc/haproxy/haproxy.cfg:ro
      - ./certs:/etc/ssl/certs:ro
    ports:
      - "80:80"
      - "443:443"
    depends_on:
      - backend-1
      - backend-2
      - backend-3
    networks:
      - backend-net

  # ===== Monitoring =====
  prometheus:
    image: prom/prometheus:latest
    volumes:
      - ./prometheus.yml:/etc/prometheus/prometheus.yml
      - prometheus-data:/prometheus
    ports:
      - "9090:9090"
    networks:
      - backend-net

  grafana:
    image: grafana/grafana:latest
    volumes:
      - grafana-data:/var/lib/grafana
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=${GRAFANA_PASSWORD}
    networks:
      - backend-net

networks:
  backend-net:
    driver: bridge

volumes:
  redis-master-data:
  redis-replica1-data:
  redis-replica2-data:
  prometheus-data:
  grafana-data:
```

---

## Deployment Checklist

### Pre-Deployment

- [ ] **DNS configured**
  - [ ] A records for all services
  - [ ] SSL certificates obtained (Let's Encrypt)
  - [ ] CDN configured (CloudFlare/AWS CloudFront)

- [ ] **Infrastructure provisioned**
  - [ ] 3 backend servers (4 vCPU, 8GB RAM each)
  - [ ] 3 Redis nodes (2 vCPU, 4GB RAM each)
  - [ ] 3 LiveKit nodes (8 vCPU, 16GB RAM each)
  - [ ] 3 coturn servers (4 vCPU, 8GB RAM, US/EU/Asia)
  - [ ] 2 HAProxy servers (2 vCPU, 4GB RAM, active-passive)
  - [ ] 1 monitoring server (4 vCPU, 8GB RAM)

- [ ] **Security configured**
  - [ ] Firewall rules (allow only necessary ports)
  - [ ] VPC/private network setup
  - [ ] SSH key-based auth (no passwords)
  - [ ] Secrets in environment variables (not hardcoded)

### Deployment Steps

1. **Deploy Redis Cluster**
   ```bash
   docker-compose up -d redis-master redis-replica-1 redis-sentinel-1
   ```

2. **Verify Redis Cluster**
   ```bash
   redis-cli -h redis-master ping
   redis-cli -h redis-master info replication
   ```

3. **Deploy Backend Instances**
   ```bash
   docker-compose up -d backend-1 backend-2 backend-3
   ```

4. **Deploy LiveKit Cluster**
   ```bash
   docker-compose up -d livekit-1 livekit-2 livekit-3
   ```

5. **Deploy HAProxy**
   ```bash
   docker-compose up -d haproxy
   ```

6. **Deploy Monitoring**
   ```bash
   docker-compose up -d prometheus grafana
   ```

7. **Deploy coturn (on separate VMs)**
   ```bash
   # On each TURN server
   sudo systemctl start coturn
   ```

### Post-Deployment Verification

- [ ] **Health checks**
  - [ ] Backend: `curl https://api.sohbet.com/health`
  - [ ] LiveKit: `curl http://livekit-1:7881/health`
  - [ ] Redis: `redis-cli ping`
  - [ ] TURN: `turnutils_uclient -v turn.sohbet.com`

- [ ] **Monitoring**
  - [ ] Prometheus targets up: http://prometheus:9090/targets
  - [ ] Grafana dashboards: http://grafana:3000
  - [ ] Alerts configured and firing (test with mock failure)

- [ ] **Load testing**
  - [ ] Simulate 100 concurrent users
  - [ ] Verify session affinity (same user → same backend)
  - [ ] Verify LiveKit room distribution
  - [ ] Test migration scenarios (P2P → SFU)

### Rollback Plan

If deployment fails:

1. **DNS rollback** (point to old infrastructure)
2. **Database restore** (from Redis backup)
3. **Notify users** (maintenance window)
4. **Post-mortem** (what went wrong?)

---

## Revised Total Cost

### Infrastructure Costs (Production-Grade with All Components)

| Component | Instances | Cost/Instance | Monthly Total |
|-----------|-----------|---------------|---------------|
| Backend Servers | 3 | $50 | $150 |
| Redis Cluster | 3 | $30 | $90 |
| LiveKit SFU | 3 | $150 | $450 |
| coturn TURN | 3 | $500 | $1,500 |
| HAProxy LB | 2 | $30 | $60 |
| Monitoring | 1 | $50 | $50 |
| **Subtotal Infrastructure** | - | - | **$2,300** |
| Bandwidth (5TB) | - | - | $450 |
| **Total** | - | - | **$2,750/month** |

### Cost per User (1000 active users)

- Infrastructure: $2,750 / 1000 = **$2.75/user/month**
- TURN bandwidth: $0.32/user/month (50% usage)
- **Total: $3.07/user/month**

### Comparison

| Approach | Monthly Cost | Notes |
|----------|--------------|-------|
| **P2P Only (original)** | $0 | Impossible for >25 users |
| **Hybrid (v1.0 estimate)** | $200 | No redundancy, public TURN |
| **Hybrid (v2.0 estimate)** | $830 | With HA, no private TURN |
| **Hybrid (v2.1 ACTUAL)** | **$2,750** | Full production with TURN |

**Reality Check:** Production costs 13.75x the original optimistic estimate.

---

## Summary

This addendum addressed critical production gaps:

1. [COMPLETE] **WebSocket Distribution**
   - HAProxy with sticky sessions (source IP hash)
   - Redis Pub/Sub for cross-instance messaging
   - Instance health monitoring with automatic cleanup

2. [COMPLETE] **LiveKit Clustering**
   - 3-node cluster with Redis coordination
   - Consistent hashing for room assignment
   - Node health monitoring and room migration

3. [COMPLETE] **Browser Compatibility**
   - Safari AudioContext unlock mechanism
   - Codec negotiation (Opus fallback)
   - Mobile battery/performance management
   - iOS-specific fixes

4. [COMPLETE] **Private TURN Infrastructure**
   - coturn self-hosted in 3 regions
   - Geographic routing via HAProxy
   - Time-limited credentials
   - $1,500/month additional cost

**Bottom Line:**
- Original estimate (v1.0): $200/month
- Critical fixes estimate (v2.0): $830/month
- **Actual production cost (v2.1): $2,750/month**

**This is the real cost of a production WebRTC hybrid system.**

---

**Document Version History**

| Version | Date | Changes |
|---------|------|---------|
| 2.1 | 2025-11-09 | Production deployment details |
| 2.0 | 2025-11-09 | Critical fixes |
| 1.0 | 2025-11-09 | Original architecture |

---

**End of Production Deployment Guide**
