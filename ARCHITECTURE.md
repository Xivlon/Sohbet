# Sohbet - Comprehensive Architecture Overview

## Project Information
- **Version**: 0.3.0-academic
- **Repository**: https://github.com/Xivlon/Sohbet.git
- **Working Directory**: /home/user/Sohbet
- **Current Branch**: claude/websockets-webrtc-logging-011CV6378CS1H4VfkXypMfCn

---

## 1. FLY.IO CONFIGURATION

### Location
**File**: `/home/user/Sohbet/backend/fly.toml` (Lines 1-55)

### Configuration Details
```
App Name: sohbet-uezxqq
Primary Region: iad (US East - Virginia)

Services:
1. HTTP API Service (Port 8080)
   - Internal Port: 8080
   - Handlers: HTTP on port 80, TLS/HTTPS on port 443
   - Health Check: GET /api/status (10s interval, 5s timeout)
   - Auto-scaling: min_machines_running = 1
   
2. WebSocket Service (Port 8081)
   - Internal Port: 8081
   - Handler: TLS on port 8081
   - Processes: Both services run on same "app" processes
   - Auto-scaling: min_machines_running = 1

VM Configuration:
   - Memory: 1GB
   - CPU: shared (1 core)
   - Memory: 1024MB

Database:
   - Uses external Neon PostgreSQL (not embedded)
   - Configuration via environment variables:
     * DATABASE_URL (connection string)
     * SOHBET_JWT_SECRET (authentication)
```

### Environment Variables Required for Fly.io
- `DATABASE_URL`: PostgreSQL connection string
- `SOHBET_JWT_SECRET`: JWT signing secret
- `NEXT_PUBLIC_API_URL`: Frontend API endpoint
- `NEXT_PUBLIC_WS_URL`: Frontend WebSocket endpoint (wss://)

---

## 2. WEBSOCKET IMPLEMENTATION

### Server-Side (Backend - C++)

#### Location
- **Header**: `/home/user/Sohbet/include/server/websocket_server.h` (159 lines)
- **Implementation**: `/home/user/Sohbet/src/server/websocket_server.cpp` (602 lines)

#### WebSocket Server Features
- Custom C++ implementation using raw sockets
- Port: Configurable via `WS_PORT` environment variable (default: 8081)
- Authentication: Token-based via query parameter (?token=...)
- Message Format: JSON with type and payload
- Thread-safe: Uses mutex locks for concurrent access
- Frame Handling: Proper WebSocket frame encoding/decoding with base64

#### Message Types Supported
```
Chat Messages:
  - chat:send
  - chat:message
  - chat:typing

User Status:
  - user:online
  - user:offline

Voice Signaling:
  - voice:join
  - voice:leave
  - voice:offer
  - voice:answer
  - voice:ice-candidate
  - voice:mute
  - voice:video-toggle
  - voice:user-joined
  - voice:user-left
  - voice:participants
  - voice:user-muted
  - voice:user-video-toggled
```

#### Key Classes
```cpp
// WebSocket Message Structure
struct WebSocketMessage {
    std::string type;
    std::string payload;
};

// WebSocket Connection
class WebSocketConnection {
    int socket_fd_;
    int user_id_;
    bool authenticated_;
    std::mutex send_mutex_;  // Thread-safe sending
    bool sendMessage(const std::string& message);
};

// WebSocket Server
class WebSocketServer {
    int port_;
    std::map<int, std::shared_ptr<WebSocketConnection>> connections_;
    std::map<int, std::set<int>> user_sockets_;
    std::map<std::string, MessageHandler> handlers_;
    
    Methods:
    - start()
    - stop()
    - registerHandler(type, handler)
    - sendToUser(user_id, message)
    - sendToUsers(user_ids, message)
    - broadcast(message)
    - isUserOnline(user_id)
    - getOnlineUsers()
};
```

#### Server Initialization (main.cpp, Lines 1-83)
```cpp
int main() {
    // Get HTTP port from environment (default 8080)
    int http_port = sohbet::config::get_http_port();
    
    // Create server with PostgreSQL
    sohbet::server::AcademicSocialServer server(http_port, database_url);
    
    // Initialize (setup DB, run migrations)
    server.initialize();
    
    // Start HTTP and WebSocket servers
    server.start();
}
```

### Client-Side (Frontend - TypeScript/React)

#### Location
**File**: `/home/user/Sohbet/frontend/app/lib/websocket-service.ts` (461 lines)

#### WebSocket Service Features
- Singleton pattern implementation
- Automatic reconnection with exponential backoff (5 attempts)
- Multiple data format support (Blob, ArrayBuffer, String)
- Robust message parsing for concatenated messages
- Event-based handler system (on/off pattern)
- Connection status monitoring

#### Configuration
```typescript
// Environment-based URL configuration
constructor(url?: string) {
    const envWsUrl = process.env.NEXT_PUBLIC_WS_URL;
    
    if (url) {
        this.url = url;
    } else if (envWsUrl) {
        // Convert https:// to wss://, http:// to ws://
        this.url = envWsUrl.replace(/^https:\/\//, 'wss://')
                           .replace(/^http:\/\//, 'ws://');
    } else {
        // Fallback: localhost:8081
        const hostname = window.location.hostname;
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        this.url = `${protocol}//${hostname}:8081`;
    }
}

Default URL (from config.ts, Line 2):
  'wss://sohbet-uezxqq.fly.dev:8081'
```

#### Key Methods
```typescript
// Connection Management
async connect(token: string): Promise<void>
disconnect(): void
isConnected(): boolean
onConnectionChange(listener): unsubscribe function

// Message Sending
send(type: WebSocketMessageType, payload: MessagePayload): boolean
sendChatMessage(conversationId, content): boolean
sendTypingIndicator(conversationId): boolean

// Message Handlers
on(type: string, handler: MessageHandler): void
off(type: string, handler: MessageHandler): void

// Data Processing
private processWebSocketData(rawData: string): void
private parseMultipleMessages(rawData: string): WebSocketMessage[]
```

#### Reconnection Strategy
- Max attempts: 5
- Exponential backoff: `delay = 1000 * 2^(attempt-1)`
- Automatic retry on connection failure
- No reconnection without valid token

---

## 3. MUMBLE/MURMUR SERVER INTEGRATION

### Status
**NOT YET INTEGRATED** - Using stub implementation for development/testing

### Configuration Files

#### Voice Config Header
**File**: `/home/user/Sohbet/include/voice/voice_config.h` (42 lines)

```cpp
struct VoiceConfig {
    bool enabled;                      // Enable/disable voice
    std::string murmur_host;           // Murmur server hostname
    int murmur_port;                   // Murmur port (default: 64738)
    std::string murmur_admin_password; // Admin password
    int token_expiry_seconds;          // Token validity period
    int max_users_per_channel;         // Users per channel limit
    bool enable_recording;             // Recording capability
};
```

#### Voice Config Implementation
**File**: `/home/user/Sohbet/src/voice/voice_config.cpp` (97 lines)

```cpp
Environment Variables Used:
- SOHBET_VOICE_ENABLED: true/1 to enable voice
- SOHBET_MURMUR_HOST: Murmur server hostname
  * Auto-detects Fly.io: {FLY_APP_NAME}.fly.dev
  * Fallback: PUBLIC_HOSTNAME environment variable
- SOHBET_MURMUR_PORT: Murmur server port (default: 64738)
- SOHBET_MURMUR_ADMIN_PASSWORD: Admin authentication
- SOHBET_VOICE_TOKEN_EXPIRY: Token expiry in seconds (default: 300)
- SOHBET_VOICE_MAX_USERS: Max users per channel (default: 25)
- SOHBET_VOICE_ENABLE_RECORDING: Enable recording (default: false)
```

### Voice Service Implementation

#### Header
**File**: `/home/user/Sohbet/include/voice/voice_service.h` (134 lines)

```cpp
// Token structure for voice connections
struct VoiceConnectionToken {
    std::string token;
    std::string murmur_host;
    int murmur_port;
    std::time_t expires_at;
    
    std::string to_json() const;
};

// Abstract base class
class VoiceService {
    virtual VoiceChannel create_channel(...) = 0;
    virtual bool delete_channel(int channel_id) = 0;
    virtual std::vector<VoiceChannel> list_channels() = 0;
    virtual VoiceChannel get_channel(int channel_id) = 0;
    virtual VoiceConnectionToken generate_connection_token(...) = 0;
    virtual bool validate_token(const std::string& token) = 0;
    virtual void log_access(int user_id, int channel_id, ...) = 0;
};

// Stub implementation (in-memory, for testing)
class VoiceServiceStub : public VoiceService {
    std::vector<VoiceChannel> channels_;
    int next_channel_id_;
};
```

#### Voice Service Source
**File**: `/home/user/Sohbet/src/voice/voice_service.cpp` (121 lines)

#### Voice Model
**File**: `/home/user/Sohbet/include/models/voice_channel.h`

```cpp
struct VoiceChannel {
    int id;
    std::string name;
    std::string channel_type;  // 'private', 'group', 'public'
    int group_id;
    int organization_id;
    std::string murmur_channel_id;
    std::time_t created_at;
};
```

### Current Implementation Status
- **Stub Service**: VoiceServiceStub used instead of real Murmur integration
- **In-Memory Storage**: Channels stored in memory during runtime
- **Token Generation**: Simple hash-based token (non-cryptographic)
- **No Real Murmur Connection**: Awaiting full Murmur integration

### Integration Directory
**Path**: `/home/user/Sohbet/backend/murmur_server_integration/`

Contains placeholder files for future Murmur integration:
- `/server` - Server connection code
- `/voice` - Voice-specific handlers
- Other directories for database and utility functions

---

## 4. WEBRTC IMPLEMENTATION

### Client-Side WebRTC Service

#### Location
**File**: `/home/user/Sohbet/frontend/app/lib/webrtc-service.ts` (1,250 lines)

#### ICE Servers Configuration (Lines 29-66)
```typescript
Default ICE Servers (optimized for 4 servers < 5-server warning):
1. Google STUN (primary):
   - stun:stun.l.google.com:19302
   - stun:stun1.l.google.com:19302

2. Twilio STUN:
   - stun:global.stun.twilio.com:3478

3. Cloudflare STUN:
   - stun:stun.cloudflare.com:3478

4. Twilio TURN (with credentials):
   - turn:global.turn.twilio.com:3478?transport=udp
   - turn:global.turn.twilio.com:3478?transport=tcp
   - turn:global.turn.twilio.com:443?transport=tcp

5. Metered TURN (backup):
   - turn:a.relay.metered.ca:80/443 (TCP/UDP)

6. OpenRelay (fallback):
   - turn:openrelay.metered.ca:443?transport=tcp
```

#### Peer Connection Configuration (Lines 804-810)
```cpp
RTCPeerConnection config:
{
    iceServers: DEFAULT_ICE_SERVERS,
    iceTransportPolicy: 'all',      // Try all connection types
    bundlePolicy: 'max-bundle',     // Single transport for better NAT
}
```

#### Key Classes & Interfaces

```typescript
// Participant information
interface VoiceParticipant {
    userId: number;
    username: string;
    university: string;
    isMuted: boolean;
    isVideoEnabled: boolean;
    isSpeaking: boolean;
    audioLevel: number;
    stream?: MediaStream;
}

// Main WebRTC Service
class WebRTCService {
    private peerConnections: Map<number, RTCPeerConnection>
    private participants: Map<number, VoiceParticipant>
    private audioAnalyzers: Map<number, AnalyserNode>
    private audioGainNodes: Map<number, GainNode>
    private pendingIceCandidates: Map<number, RTCIceCandidateInit[]>
    private iceRestartAttempts: Map<number, number>
    private connectionFailureTimeouts: Map<number, NodeJS.Timeout>
}
```

#### Core Methods

```typescript
// Channel Management
async joinChannel(channelId: number, userId: number, audioOnly?: boolean)
leaveChannel()

// Media Controls
toggleMute(): boolean
async toggleVideo(): Promise<boolean>
toggleDeafen(): boolean
setParticipantVolume(userId: number, volume: number)

// Peer Connection
private createOffer(targetUserId: number)
private handleOffer(payload: any)
private handleAnswer(payload: any)
private handleIceCandidate(payload: any)
private restartIce(userId: number)
private closePeerConnection(userId: number)

// Audio Analysis
private setupAudioAnalyzer()
private setupRemoteAudioAnalyzer(userId: number, stream: MediaStream)
private monitorAudioLevel(userId: number, analyzer: AnalyserNode)

// Connection Quality
private monitorConnectionQuality(userId: number, pc: RTCPeerConnection)
private updateConnectionQuality()
```

#### WebSocket Handler Integration (Lines 99-256)

Integrates with WebSocket service for signaling:
```typescript
// Handlers registered for voice signaling messages
websocketService.on('voice:user-joined', handler)
websocketService.on('voice:user-left', handler)
websocketService.on('voice:participants', handler)
websocketService.on('voice:offer', handler)
websocketService.on('voice:answer', handler)
websocketService.on('voice:ice-candidate', handler)
websocketService.on('voice:user-muted', handler)
websocketService.on('voice:user-video-toggled', handler)
```

#### Advanced Features

**Offer Collision Handling (Lines 654-675)**
- Implements polite/impolite pattern
- Lower user ID is "polite" and rolls back
- Prevents simultaneous offer conflicts

**ICE Restart Strategy (Lines 867-931)**
- Auto-restart on ICE connection failure
- Max 3 restart attempts per user
- Fallback: Close and recreate connection after 2 seconds
- Timeout-based disconnection handling

**Audio Analysis (Lines 554-590)**
- Real-time speaking detection
- Audio level calculation (0-1 normalized)
- Speaking threshold: 0.01

**Volume Control (Lines 500-527)**
- Per-participant volume control (0-1)
- GainNode-based audio level control
- Persistent across stream changes

---

## 5. SERVER ARCHITECTURE

### Backend Technology Stack
- **Language**: C++17
- **Build System**: CMake 3.16+
- **Database**: PostgreSQL (via libpqxx)
- **HTTP Server**: Custom C++ implementation
- **Authentication**: JWT with bcrypt password hashing
- **Concurrency**: Multi-threaded with mutex locks
- **Serialization**: nlohmann/json

### Main Server Implementation

#### Location
**File**: `/home/user/Sohbet/src/server/server.cpp` (4,360 lines)

#### Server Ports

From `/home/user/Sohbet/include/config/env.h` (Lines 34-48):
```cpp
HTTP Port:      Default 8080 (from PORT env var)
WebSocket Port: Default 8081 (from WS_PORT env var)
```

#### Server Initialization (Server.cpp, Lines 54-168)

```cpp
AcademicSocialServer::AcademicSocialServer(int port, string connection_string)

Initialize Process:
1. Create PostgreSQL database connection
2. Create all repositories:
   - UserRepository
   - VoiceChannelRepository
   - ConversationRepository
   - MessageRepository
   - PostRepository
   - CommentRepository
   - GroupRepository
   - OrganizationRepository
   - And 13+ more data repositories
3. Initialize services:
   - VoiceServiceStub (for voice channels)
   - StorageService (file uploads)
   - EmailService (if libcurl available)
   - StudyBuddyMatchingService
4. Initialize WebSocket server on port 8081
5. Run database migrations:
   - User and auth tables
   - Social features (posts, comments, friendships)
   - Enhanced features
   - Study buddy matching

Port Configuration (Lines 23-47):
- HTTP_PORT environment variable (default: 8080)
- WS_PORT environment variable (default: 8081)
- Both use PORT environment variable as fallback
```

#### Server Start (Lines 170-220+)

```cpp
bool AcademicSocialServer::start()

Actions:
1. Initialize server socket on HTTP port
2. Start WebSocket server on WS_PORT
3. Register WebSocket message handlers:
   - Chat message handler
   - Typing indicator handler
   - Voice join/leave handlers
   - Voice offer/answer handlers
   - ICE candidate handler
   - Mute/video toggle handlers
4. Start voice channel cleanup thread
5. Listen for HTTP requests
6. Output available endpoints
```

### Voice Channel Handlers (Server.cpp, Lines 2918-3300+)

#### Handler Registration (Lines 2918-2960)
```cpp
void AcademicSocialServer::setupWebSocketHandlers() {
    websocket_server_->registerHandler("chat:send", ...);
    websocket_server_->registerHandler("chat:typing", ...);
    websocket_server_->registerHandler("voice:join", ...);
    websocket_server_->registerHandler("voice:leave", ...);
    websocket_server_->registerHandler("voice:offer", ...);
    websocket_server_->registerHandler("voice:answer", ...);
    websocket_server_->registerHandler("voice:ice-candidate", ...);
    websocket_server_->registerHandler("voice:mute", ...);
    websocket_server_->registerHandler("voice:video-toggle", ...);
}
```

#### Voice Join Handler (handleVoiceJoin, ~50 lines)
- Extracts channel_id from WebSocket message
- Validates user exists
- Adds user to voice_channel_participants_ map
- Notifies all channel participants of new user join
- Broadcasts user joined message with user info

#### Voice Offer Handler (handleVoiceOffer, Lines 3210+)
- Receives WebRTC offer from initiating peer
- Routes offer to target user via WebSocket
- Payload contains: from_user_id, target_user_id, channel_id, offer (SDP)

#### Voice Answer Handler (handleVoiceAnswer, Lines 3250+)
- Receives WebRTC answer from responding peer
- Routes answer back to initiating peer
- Payload contains: from_user_id, target_user_id, answer (SDP)

#### ICE Candidate Handler (similar pattern)
- Receives ICE candidates during connection establishment
- Forwards to peer via WebSocket
- Payload contains: from_user_id, target_user_id, candidate (JSON)

### HTTP API Endpoints

**Status & Health**
- `GET /api/status` - Server health check

**Authentication**
- `POST /api/login` - User login

**Users**
- `GET /api/users` - List all users
- `GET /api/users/demo` - Get demo user data
- `POST /api/users` - Register new user
- `PUT /api/users/:id` - Update user profile

**Media/File Upload**
- `POST /api/media/upload` - Upload files
- `GET /api/media/file/:key` - Retrieve files

**Voice Channels** (via HTTP or WebSocket)
- Various endpoints for channel creation/management

---

## 6. CLIENT ARCHITECTURE

### Frontend Framework
- **Framework**: Next.js 16.0.0 with React 19.2.0
- **Language**: TypeScript 5.9.3
- **Build**: npm-based
- **Internationalization**: next-intl (Turkish & English)
- **Port**: 5000 (from package.json: `next dev -p 5000`)

### Frontend Structure

#### Directory Layout
```
/frontend/
├── app/
│   ├── [locale]/              # i18n routing
│   ├── api/                   # Next.js API routes
│   ├── components/            # React components
│   │   ├── websocket-provider.tsx
│   │   ├── chat-window.tsx
│   │   ├── post-card.tsx
│   │   ├── profile.tsx
│   │   └── ... (30+ more)
│   ├── contexts/              # React contexts
│   ├── lib/
│   │   ├── websocket-service.ts       # WebSocket client
│   │   ├── webrtc-service.ts          # WebRTC client
│   │   ├── voice-service.ts           # Voice API wrapper
│   │   ├── api-client.ts              # HTTP API client
│   │   ├── config.ts                  # Configuration
│   │   ├── auth.ts                    # Authentication
│   │   └── ... (utilities)
│   ├── layout.tsx
│   └── globals.css
├── package.json
├── next.config.ts
├── tsconfig.json
└── i18n.ts                    # i18n configuration
```

### Configuration (app/lib/config.ts, Lines 1-2)

```typescript
API_URL: process.env.NEXT_PUBLIC_API_URL || 'https://sohbet-uezxqq.fly.dev'
WS_URL: process.env.NEXT_PUBLIC_WS_URL || 'wss://sohbet-uezxqq.fly.dev:8081'
```

### Key Client-Side Services

#### WebSocket Service (websocket-service.ts)
- Real-time chat messaging
- Typing indicators
- User online/offline status
- Voice signaling (offers, answers, ICE candidates)

#### WebRTC Service (webrtc-service.ts)
- Voice channel management
- Peer-to-peer audio/video streaming
- Connection quality monitoring
- ICE restart and error recovery

#### Voice Service (voice-service.ts)
- API wrapper for voice channel operations
- Create/join/leave channels
- Token-based channel access

#### API Client (api-client.ts)
- HTTP requests with JWT authentication
- User management
- Post/comment operations
- Media uploads

---

## 7. DATABASE ARCHITECTURE

### Database Engine
- **Type**: PostgreSQL (external, via Neon)
- **Connection**: Via DATABASE_URL environment variable
- **ORM**: Custom C++ wrapper (libpqxx)
- **Migrations**: Programmatic SQL execution

### Key Tables
- users
- roles
- user_presence
- conversations
- messages
- voice_channels
- posts
- comments
- friendships
- groups
- organizations
- media
- notifications
- study_sessions
- And more...

### Repositories (Abstraction Layer)
Located in: `/home/user/Sohbet/include/repositories/`

Each repository provides data access for specific entities:
- UserRepository
- VoiceChannelRepository
- ConversationRepository
- MessageRepository
- GroupRepository
- OrganizationRepository
- And 10+ more

---

## 8. DEPLOYMENT & ENVIRONMENT CONFIGURATION

### Environment Variables (from .env.example)

**Authentication**
```
SOHBET_JWT_SECRET=<strong-random-secret>
JWT_SECRET=<fallback-secret>
JWT_EXPIRY_HOURS=24
```

**Database**
```
DATABASE_URL=postgresql://user:password@host:port/database
```

**API Endpoints**
```
NEXT_PUBLIC_API_URL=http://localhost:8080        (or https:// for prod)
NEXT_PUBLIC_WS_URL=ws://localhost:8081           (or wss:// for prod)
```

**Ports**
```
PORT=8080                # HTTP server port
WS_PORT=8081            # WebSocket port
```

**CORS Configuration**
```
CORS_ORIGIN=https://your-frontend-domain.com
```

**Email Service**
```
SENDGRID_API_KEY=<key>
SENDGRID_FROM_EMAIL=noreply@sohbet.app
EMAIL_VERIFICATION_ENABLED=true
EMAIL_VERIFICATION_URL=http://localhost:3000/verify-email
```

**Voice Service**
```
SOHBET_VOICE_ENABLED=false              (true to enable)
SOHBET_MURMUR_HOST=<server-hostname>    (auto-detects Fly.io)
SOHBET_MURMUR_PORT=64738                (default)
SOHBET_MURMUR_ADMIN_PASSWORD=<password>
SOHBET_VOICE_TOKEN_EXPIRY=300           (seconds)
SOHBET_VOICE_MAX_USERS=25               (per channel)
SOHBET_VOICE_ENABLE_RECORDING=false
```

### Build & Deployment

**Backend Build** (Dockerfile)
```dockerfile
FROM ubuntu:22.04 as builder
  - Install cmake, make, build tools
  - Download dependencies (PostgreSQL, OpenSSL, CURL)
  - Fetch and build libbcrypt, nlohmann/json, libpqxx

FROM ubuntu:22.04 (runtime)
  - Copy compiled binary from builder
  - Copy migrations
  - Expose ports 8080 (HTTP), 8081 (WebSocket)
  - Run: ./sohbet
```

**Frontend Deployment**
- Vercel (recommended): Deploy from `/frontend` directory
- Custom hosting: Run `npm run build && npm start`
- Port: 5000

**Fly.io Deployment**
- Uses fly.toml configuration
- Auto-scaling with health checks
- Database: External Neon PostgreSQL
- TLS/HTTPS automatically configured

---

## 9. KEY ARCHITECTURAL PATTERNS

### 1. Repository Pattern
All data access goes through repositories (UserRepository, VoiceChannelRepository, etc.)

### 2. Service Layer
Business logic in service classes (VoiceService, StorageService, EmailService)

### 3. Middleware Architecture
WebSocket server uses handler registration pattern for message processing

### 4. Singleton Pattern
- WebSocketService (client-side)
- WebRTCService (client-side)
- Voice repositories (server-side)

### 5. Observer Pattern
- WebSocket event handlers (on/off pattern)
- WebRTC connection quality callbacks
- Participant update notifications

### 6. Factory Pattern
WebSocketServer creates and manages WebSocketConnection instances

---

## 10. SECURITY CONSIDERATIONS

### Authentication
- JWT tokens with bcrypt password hashing
- Token expiry: Configurable (default 24 hours)
- Secure token transmission via WebSocket

### Data Protection
- SSL/TLS for all connections
- HTTPS/WSS in production
- Input validation and JSON escaping
- Rate limiting support (infrastructure)

### WebSocket Security
- Origin header validation (regex-based)
- Header injection prevention
- Base64 encoding for frame headers
- Thread-safe socket operations

---

## 11. PERFORMANCE OPTIMIZATIONS

### Backend
- Multi-threaded socket handling
- Connection pooling for database
- Mutex-based synchronous locking
- Efficient JSON parsing

### Client
- WebRTC for direct P2P (reducing server load)
- Exponential backoff for reconnection
- Audio analysis optimization (256-sample FFT)
- Memory cleanup on channel leave

### Network
- WebSocket for real-time messaging
- Bundled media over single transport
- ICE candidate batching
- Automatic connection recovery

---

## 12. MONITORING & LOGGING

### Server-Side Logging
- Standard output for startup events
- Error logging to stderr
- Voice channel participant tracking
- Connection state transitions

### Client-Side Logging
- Debug mode support (isDebugEnabled function)
- WebSocket connection logs
- WebRTC peer connection state logs
- Audio level monitoring

---

## SUMMARY

**Sohbet** is a full-stack Turkish academic social platform built with:
- **Backend**: C++17 with PostgreSQL
- **Frontend**: Next.js 16 with React 19
- **Real-time**: WebSocket (ports 8080/8081)
- **P2P Voice**: WebRTC with fallback STUN/TURN servers
- **Deployment**: Fly.io with Neon PostgreSQL
- **Scalability**: Auto-scaling, health checks, external database
- **Future**: Murmur server integration for voice (currently stubbed)

The architecture is well-designed with clear separation of concerns, using repositories for data access, services for business logic, and proper error handling throughout.
