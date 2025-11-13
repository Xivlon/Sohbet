# Sohbet Architecture - Quick Reference Guide

## Key File Locations

### Configuration
| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| Fly.io Config | `/backend/fly.toml` | 1-55 | Deployment config for Fly.io |
| Environment Config | `/include/config/env.h` | 1-73 | Environment variable helpers |
| Frontend Config | `/frontend/app/lib/config.ts` | 1-3 | API/WS URLs |

### WebSocket Implementation
| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| WS Server Header | `/include/server/websocket_server.h` | 1-159 | WebSocket server interface |
| WS Server Impl | `/src/server/websocket_server.cpp` | 1-602 | WebSocket server implementation |
| WS Client | `/frontend/app/lib/websocket-service.ts` | 1-461 | Client WebSocket service |

### Voice/Mumble Integration
| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| Voice Config Header | `/include/voice/voice_config.h` | 1-42 | Voice configuration interface |
| Voice Config Impl | `/src/voice/voice_config.cpp` | 1-97 | Voice configuration implementation |
| Voice Service Header | `/include/voice/voice_service.h` | 1-134 | Voice service interface |
| Voice Service Impl | `/src/voice/voice_service.cpp` | 1-121 | Voice service (stub) implementation |
| Voice Channel Model | `/include/models/voice_channel.h` | - | Voice channel data model |

### WebRTC Implementation
| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| WebRTC Service | `/frontend/app/lib/webrtc-service.ts` | 1-1250 | Client WebRTC service |
| ICE Servers | `/frontend/app/lib/webrtc-service.ts` | 29-66 | STUN/TURN configuration |
| Peer Connection | `/frontend/app/lib/webrtc-service.ts` | 804-810 | RTCPeerConnection config |
| Offer Handling | `/frontend/app/lib/webrtc-service.ts` | 646-714 | Handle incoming WebRTC offers |
| Answer Handling | `/frontend/app/lib/webrtc-service.ts` | 719-737 | Handle incoming WebRTC answers |
| ICE Candidate | `/frontend/app/lib/webrtc-service.ts` | 742-793 | Handle ICE candidates |

### Server Architecture
| Component | File | Lines | Purpose |
|-----------|------|-------|---------|
| Main Entry | `/src/main.cpp` | 1-83 | Server entry point |
| Server Header | `/include/server/server.h` | 1-100+ | Server interface |
| Server Impl | `/src/server/server.cpp` | 1-4360 | Main server implementation |
| Server Init | `/src/server/server.cpp` | 54-168 | Server initialization |
| Server Start | `/src/server/server.cpp` | 170-220 | Server startup |
| WS Handlers | `/src/server/server.cpp` | 2918-2960 | WebSocket handler registration |
| Voice Join | `/src/server/server.cpp` | ~3040 | Handle voice:join messages |
| Voice Offer | `/src/server/server.cpp` | 3210+ | Handle voice:offer messages |
| Voice Answer | `/src/server/server.cpp` | 3250+ | Handle voice:answer messages |

### Frontend Architecture
| Component | File | Purpose |
|-----------|------|---------|
| API Client | `/frontend/app/lib/api-client.ts` | HTTP API client with JWT auth |
| Voice Service | `/frontend/app/lib/voice-service.ts` | Voice channel operations wrapper |
| Auth Helper | `/frontend/app/lib/auth.ts` | Authentication utilities |
| Next Config | `/frontend/next.config.ts` | Next.js build configuration |
| Main Layout | `/frontend/app/layout.tsx` | Root layout component |
| WebSocket Provider | `/frontend/app/components/websocket-provider.tsx` | WebSocket context provider |

### Build & Deployment
| File | Purpose |
|------|---------|
| `/CMakeLists.txt` | C++ build configuration |
| `/Dockerfile` | Docker image for backend |
| `/frontend/package.json` | Frontend dependencies |
| `/env.deployment` | Deployment environment example |
| `.env.example` | Environment variables template |

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         FLY.IO DEPLOYMENT                        │
│  (sohbet-uezxqq.fly.dev)                                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌───────────────────┐           ┌──────────────────────┐       │
│  │  Frontend (React) │           │  Backend (C++)       │       │
│  │  Port: 5000       │  ├──────>│  Port: 8080 (HTTP)   │       │
│  │  (Vercel/Netlify) │           │  Port: 8081 (WS)     │       │
│  └───────────────────┘           └──────────────────────┘       │
│         ▲                              │        │                │
│         │ HTTP/REST                   │        │                │
│         │ WebSocket                   │        └─── PostgreSQL  │
│         │ (wss://)                    │            (Neon)       │
│         │                             │                         │
│  ┌──────┴──────────────────────────────────────────────────┐    │
│  │              CLIENT SERVICES                             │    │
│  │  • websocket-service.ts (WebSocket signaling)           │    │
│  │  • webrtc-service.ts (P2P voice/video)                 │    │
│  │  • voice-service.ts (Voice channel API wrapper)        │    │
│  │  • api-client.ts (REST API client)                     │    │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │           SERVER-SIDE ARCHITECTURE                       │    │
│  │  • WebSocketServer (C++) - Port 8081                    │    │
│  │    - Message handler routing                            │    │
│  │    - Connection management                              │    │
│  │    - Concurrent socket handling                         │    │
│  │                                                          │    │
│  │  • HTTP Server (C++) - Port 8080                        │    │
│  │    - RESTful API endpoints                              │    │
│  │    - JWT authentication                                 │    │
│  │    - Request routing                                    │    │
│  │                                                          │    │
│  │  • Voice Service (Stub) - For future Murmur integration│    │
│  │    - Channel management                                 │    │
│  │    - Token generation                                   │    │
│  │    - Access logging                                     │    │
│  │                                                          │    │
│  │  • Database Layer                                        │    │
│  │    - 20+ repositories (data access)                     │    │
│  │    - PostgreSQL connection                              │    │
│  │    - Migrations runner                                  │    │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │         VOICE/VIDEO SIGNALING (WebSocket)               │    │
│  │  Messages: voice:join, voice:leave                      │    │
│  │            voice:offer, voice:answer                    │    │
│  │            voice:ice-candidate                          │    │
│  │  Routing: Server broadcasts to channel participants    │    │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │     PEER-TO-PEER VOICE (WebRTC - Outside Fly.io)       │    │
│  │  Direct connections between users                       │    │
│  │  STUN servers: Google, Twilio, Cloudflare              │    │
│  │  TURN servers: Twilio, Metered, OpenRelay              │    │
│  │  ICE restart on failure (max 3 attempts)               │    │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Port Mapping

| Port | Service | Protocol | Location |
|------|---------|----------|----------|
| 5000 | Frontend | HTTP | Next.js dev/prod |
| 8080 | HTTP API | HTTP/HTTPS | Backend (C++) |
| 8081 | WebSocket | WS/WSS | Backend (C++) |
| 64738 | Murmur | Custom | Not integrated yet |

## Message Flow - Voice Call Example

```
User A Joins Channel:
1. User A connects WebSocket to backend:8081
   → sends: {type: "voice:join", payload: {channel_id: 5}}
   
2. Backend broadcasts to all participants in channel:
   {type: "voice:user-joined", payload: {user_id: 1, username: "alice", ...}}

User B Initiates Offer:
3. User B's browser (WebRTC Service) creates offer
   → sends via WebSocket: {type: "voice:offer", payload: {
       from_user_id: 2,
       target_user_id: 1,
       offer: {type: "offer", sdp: "..."}
     }}

4. Backend routes to User A via WebSocket

User A Receives & Responds:
5. User A's browser receives offer, creates answer
   → sends: {type: "voice:answer", payload: {
       from_user_id: 1,
       target_user_id: 2,
       answer: {type: "answer", sdp: "..."}
     }}

ICE Gathering (Parallel):
6. Both browsers send ICE candidates as they're gathered:
   → {type: "voice:ice-candidate", payload: {
       from_user_id: X,
       target_user_id: Y,
       candidate: {...}
     }}

Connection Established:
7. P2P audio/video stream established between browsers
8. No more signaling needed unless connection fails
9. Audio analyzed for speaking detection & level
10. Volume control available per participant
```

## Key Metrics & Defaults

| Setting | Value | Configurable |
|---------|-------|--------------|
| HTTP Port | 8080 | PORT env var |
| WebSocket Port | 8081 | WS_PORT env var |
| JWT Expiry | 24 hours | JWT_EXPIRY_HOURS |
| Token Expiry (Voice) | 300 seconds | SOHBET_VOICE_TOKEN_EXPIRY |
| Max Users/Channel | 25 | SOHBET_VOICE_MAX_USERS |
| Reconnect Attempts | 5 | hardcoded in client |
| Reconnect Backoff | exponential 1s-16s | 1000 * 2^(n-1) |
| ICE Restart Limit | 3 | MAX_ICE_RESTART_ATTEMPTS |
| Audio FFT Size | 256 | webrtc-service.ts:539 |
| Speaking Threshold | 0.01 | webrtc-service.ts:578 |
| WS Health Check | 10s interval | fly.toml:26 |

## Technologies & Dependencies

**Backend (C++17)**
- CMake 3.16+ (build)
- PostgreSQL + libpqxx (database)
- OpenSSL (crypto/TLS)
- nlohmann/json (JSON)
- bcrypt (password hashing)
- CURL (email service, optional)

**Frontend (Node.js/TypeScript)**
- Next.js 16.0.0
- React 19.2.0
- TypeScript 5.9.3
- Tailwind CSS 4
- Radix UI (components)
- next-intl (i18n)

**Deployment**
- Fly.io (hosting)
- Neon (PostgreSQL)
- Vercel (optional frontend)
- Docker (containerization)

## Environment Variables Summary

```bash
# Must Set (Production)
SOHBET_JWT_SECRET=<random-strong-secret>
DATABASE_URL=postgresql://...

# Should Set (Production)
NEXT_PUBLIC_API_URL=https://api.yourdomain.com
NEXT_PUBLIC_WS_URL=wss://api.yourdomain.com:8081
CORS_ORIGIN=https://your-frontend.com

# Voice (Currently Stubbed)
SOHBET_VOICE_ENABLED=false
SOHBET_MURMUR_HOST=<optional>
SOHBET_MURMUR_PORT=64738
SOHBET_MURMUR_ADMIN_PASSWORD=<optional>

# Optional
PORT=8080
WS_PORT=8081
JWT_EXPIRY_HOURS=24
SENDGRID_API_KEY=<optional>
```

## Status Summary

| Feature | Status | Location |
|---------|--------|----------|
| WebSocket | ✓ Complete | `/include/server/websocket_server.h` |
| WebRTC P2P Voice | ✓ Complete | `/frontend/app/lib/webrtc-service.ts` |
| Chat Messaging | ✓ Complete | WebSocket handlers |
| Voice Channels | ✓ Stubbed | VoiceServiceStub |
| Murmur Integration | ⚠ Not Started | `/backend/murmur_server_integration/` |
| User Management | ✓ Complete | UserRepository |
| Authentication | ✓ Complete | JWT + bcrypt |
| Database | ✓ Complete | PostgreSQL (Neon) |
| Deployment | ✓ Complete | Fly.io configuration |

