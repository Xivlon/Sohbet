# Voice and Video Integration Guide

## Overview

This document describes how Sohbet integrates voice and video calling capabilities using **WebRTC** (Web Real-Time Communication). This integration is designed with security and usability as top priorities, requiring no external applications or plugins for end users.

## WebRTC Architecture

WebRTC enables peer-to-peer voice and video communication directly in the web browser. It's perfect for Sohbet because it provides:

### Key Benefits

- **🎤 Low-Latency Voice & Video**: Crystal-clear real-time conversations with no client software needed
- **🔒 End-to-End Encryption**: All media traffic uses DTLS-SRTP encryption by default
- **👥 Permission Control**: Fine-grained control over who can access which channels
- **🌐 Browser-Native**: Works in all modern browsers without plugins
- **🔄 Automatic NAT Traversal**: ICE servers handle NAT/firewall issues automatically
- **📹 Recording Ready**: Foundation for optional recording capabilities

---

## How It Works

### Design Principles

The integration follows these important principles:

1. **🔐 Security First**: All connections are authenticated through Sohbet's existing security system
2. **🎯 Separation of Concerns**: Voice features are kept separate from core social media functionality
3. **🚀 Future-Proof**: Clean interfaces make it easy to add mobile and desktop support
4. **⚙️ Optional Service**: Voice features are optional and won't affect core functionality if disabled

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Sohbet Application                        │
├─────────────────────────────────────────────────────────────┤
│  Frontend (React)                                            │
│  ├── Voice Call UI Components (Khave)                       │
│  ├── Voice Service Client                                   │
│  └── WebRTC Peer Connection Manager                         │
├─────────────────────────────────────────────────────────────┤
│  Backend (C++)                                               │
│  ├── Voice Service Interface                                │
│  │   ├── Voice Channel Management                           │
│  │   ├── User Authentication Bridge                         │
│  │   └── Connection Token Generation (JWT)                  │
│  ├── WebSocket Server (Port 8081)                           │
│  │   ├── WebRTC Signaling                                   │
│  │   ├── SDP Offer/Answer Exchange                          │
│  │   └── ICE Candidate Relay                                │
│  └── REST API Endpoints                                     │
│      ├── POST /api/voice/channels (Create channel)          │
│      ├── GET /api/voice/channels (List channels)            │
│      ├── POST /api/voice/channels/:id/join (Join)           │
│      └── DELETE /api/voice/channels/:id (Delete channel)    │
├─────────────────────────────────────────────────────────────┤
│  External Services (Optional)                               │
│  ├── STUN Servers (ice.example.com:3478)                    │
│  │   └── NAT Type Detection & Address Discovery             │
│  └── TURN Servers (turn.example.com:3478)                   │
│      └── Relay for Strict NAT/Firewall Scenarios            │
└─────────────────────────────────────────────────────────────┘
```

### How WebRTC Signaling Works

Here's how a user securely joins a voice channel with WebRTC:

1. **Login**: User logs into Sohbet (using existing JWT authentication)
2. **Request Access**: User requests to join a voice channel
3. **Validation**: Backend checks if user has permission to join
4. **Token Generation**: System creates a JWT-based connection token
5. **Signaling Connection**: User establishes WebSocket connection to signaling server
6. **Peer Discovery**: Backend broadcasts user's presence to other participants in channel
7. **SDP Exchange**: Peers exchange Session Description Protocol offers/answers via WebSocket
8. **ICE Gathering**: Peers collect ICE candidates for NAT traversal
9. **Peer Connection**: Direct peer-to-peer connection is established
10. **Media Flow**: Audio/video streams flow directly between peers with DTLS-SRTP encryption

### Security Features

- ✅ **Token-Based Authentication**: No unauthorized access to voice channels
- ✅ **Time-Limited Tokens**: Tokens expire after 5 minutes, reducing security risks
- ✅ **Permission Validation**: Users can only join channels they're allowed to access
- ✅ **Encrypted Communications**: All voice traffic uses TLS/SSL encryption
- ✅ **Audit Logging**: All voice channel access is logged for security review
- ✅ **Rate Limiting**: Prevents abuse of token generation

---

## Implementation Progress

### ✅ Phase 1: Foundation (COMPLETED)

- [x] Architecture documentation
- [x] Security model design
- [x] Voice service interface (C++ header files)
- [x] Configuration system
- [x] Voice channel data model
- [x] Frontend TypeScript service client
- [x] React hooks for voice integration
- [x] Comprehensive testing

### ✅ Phase 2: WebRTC Core Integration (COMPLETED)

- [x] WebRTC signaling server via WebSocket (port 8081)
- [x] REST API endpoints for voice services (6 endpoints)
- [x] JWT token generation and validation
- [x] Channel management in backend
- [x] ICE server configuration
- [x] Peer connection management

### ✅ Phase 3: Frontend Integration (COMPLETED)

- [x] Voice service client (TypeScript)
- [x] WebRTC peer connection management
- [x] Khave UI for voice channels
- [x] Connection handling and error recovery
- [x] Real-time media stream handling
- [x] Voice control UI (mute/unmute/stop)

### 📋 Phase 4: Advanced Features (FUTURE)

- [ ] Screen sharing capabilities
- [ ] Recording and playback features
- [ ] Mobile native apps (iOS/Android)
- [ ] Automatic transcription services
- [ ] Group conference calling (MCU/SFU)
- [ ] Advanced audio processing (noise cancellation)

---

## Configuration Guide

### Environment Variables

To configure the WebRTC voice service, set these environment variables:

```bash
# Enable or disable voice service
export SOHBET_VOICE_ENABLED=true

# WebRTC ICE server settings (for NAT traversal)
# STUN server (free public servers available)
export SOHBET_WEBRTC_STUN_SERVER=stun:stun.l.google.com:19302

# TURN server (optional, for strict NAT/firewalls)
export SOHBET_WEBRTC_TURN_SERVER=turn:your-turn-server.com:3478
export SOHBET_WEBRTC_TURN_USERNAME=username
export SOHBET_WEBRTC_TURN_PASSWORD=password

# Token settings
export SOHBET_VOICE_TOKEN_EXPIRY=3600  # 1 hour

# Channel settings
export SOHBET_VOICE_MAX_USERS=25

# Recording (optional)
export SOHBET_VOICE_ENABLE_RECORDING=false
```

### What Each Setting Does

| Setting | Description | Default |
|---------|-------------|---------|
| `SOHBET_VOICE_ENABLED` | Turn voice features on/off | `false` |
| `SOHBET_WEBRTC_STUN_SERVER` | STUN server URL for NAT detection | `stun:stun.l.google.com:19302` |
| `SOHBET_WEBRTC_TURN_SERVER` | TURN server for relay (optional) | Not set |
| `SOHBET_WEBRTC_TURN_USERNAME` | TURN server username | Not set |
| `SOHBET_WEBRTC_TURN_PASSWORD` | TURN server password | Not set |
| `SOHBET_VOICE_TOKEN_EXPIRY` | JWT token validity in seconds | `3600` |
| `SOHBET_VOICE_MAX_USERS` | Max users per channel | `25` |
| `SOHBET_VOICE_ENABLE_RECORDING` | Allow channel recording | `false` |

---

### API Endpoints

#### Create Voice Channel
```http
POST /api/voice/channels
Authorization: Bearer <jwt_token>
Content-Type: application/json

{
  "name": "Study Group - Computer Science",
  "description": "Weekly study group meeting",
  "max_users": 10,
  "is_temporary": true
}

Response: 201 Created
{
  "channel_id": 123,
  "name": "Study Group - Computer Science",
  "murmur_channel_id": 456,
  "created_at": "2025-10-23T22:00:00Z"
}
```

#### Join Voice Channel
```http
POST /api/voice/channels/:id/join
Authorization: Bearer <jwt_token>
Content-Type: application/json

{}

Response: 200 OK
{
  "channel_id": 123,
  "connection_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "signaling_server": "wss://voice.sohbet.app:8081",
  "ice_servers": [
    {
      "urls": ["stun:stun.l.google.com:19302"]
    },
    {
      "urls": ["turn:turn.sohbet.app:3478"],
      "username": "user",
      "credential": "pass"
    }
  ],
  "expires_at": "2025-10-23T22:05:00Z"
}
```

#### List Voice Channels
```http
GET /api/voice/channels
Authorization: Bearer <jwt_token>

Response: 200 OK
{
  "channels": [
    {
      "channel_id": 123,
      "name": "Study Group - Computer Science",
      "active_users": 5,
      "max_users": 10,
      "is_temporary": true
    }
  ]
}
```

### Database Schema

New tables to support voice integration:

```sql
-- Voice channels
CREATE TABLE voice_channels (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    creator_id INTEGER NOT NULL,
    murmur_channel_id INTEGER,
    max_users INTEGER DEFAULT 25,
    is_temporary BOOLEAN DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (creator_id) REFERENCES users(id)
);

-- Voice channel access tokens
CREATE TABLE voice_tokens (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    token_hash TEXT NOT NULL UNIQUE,
    user_id INTEGER NOT NULL,
    channel_id INTEGER NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    used BOOLEAN DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (channel_id) REFERENCES voice_channels(id)
);

-- Voice channel access log
CREATE TABLE voice_access_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    channel_id INTEGER NOT NULL,
    action TEXT NOT NULL, -- 'join', 'leave', 'kicked', 'banned'
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (channel_id) REFERENCES voice_channels(id)
);
```

### Future Enhancements

1. **Mobile Support**: Native iOS and Android apps with WebRTC support
2. **Screen Sharing**: Desktop sharing for presentations
3. **Recording**: Voice channel recording with user consent
4. **Transcription**: Automatic transcription of voice conversations
5. **Group Conferencing**: Multi-party MCU (Multipoint Control Unit) or SFU (Selective Forwarding Unit)
6. **Advanced Audio Processing**: AI-powered noise cancellation and echo suppression
7. **Virtual Backgrounds**: For video calls
8. **Third-Party Integration**: Easy integration with Daily.co, Twilio, Agora

### Dependencies

WebRTC implementation requires:
- **C++ 17 Standard Library**: For peer connection management
- **WebSocket Library**: For signaling (already present in codebase)
- **OpenSSL**: For JWT token generation and DTLS encryption (already present)
- **PostgreSQL/SQLite**: For storing channel and session data (already present)
- **STUN/TURN Servers** (Optional): Public STUN servers available free, TURN servers recommended for production

### Deployment Considerations

1. **STUN Server Access**: Ensure public STUN servers are reachable from your network
2. **TURN Server Setup** (Optional): Deploy if you have strict NAT/firewall requirements
3. **WebSocket Port**: Ensure port 8081 is accessible for signaling
4. **SSL/TLS Certificates**: Required for secure WebSocket (wss://) connections
5. **Resource Planning**: Signaling server uses minimal resources (primarily I/O for WebSocket)
6. **Bandwidth**: Peer-to-peer connections use bandwidth between participants, not through server
7. **Monitoring**: Monitor WebSocket connections and signaling latency

### Testing Strategy

1. **Unit Tests**: Test individual components (token generation, validation)
2. **Integration Tests**: Test interaction between Sohbet and Murmur
3. **Load Tests**: Test capacity and performance under load
4. **Security Tests**: Test authentication and authorization
5. **End-to-End Tests**: Test full user flow from UI to voice connection

### Implementation Details

#### Backend Components

The backend implementation consists of the following key components:

**1. VoiceChannel Model** (`include/models/voice_channel.h`)
- Represents a voice channel in the system
- Supports JSON serialization/deserialization
- Tracks channel metadata (name, description, creator, capacity, etc.)

**2. VoiceConfig** (`include/voice/voice_config.h`)
- Configuration structure for voice service
- Loads configuration from environment variables
- Validates configuration settings

**3. VoiceService Interface** (`include/voice/voice_service.h`)
- Abstract interface for voice service operations
- Defines methods for channel management
- Defines token generation and validation

**4. VoiceServiceStub** (included in `voice_service.h`)
- Stub implementation for testing without Murmur server
- In-memory channel storage
- Simple token generation for development

#### Frontend Components

**1. VoiceService Class** (`frontend/src/services/voiceService.ts`)
- TypeScript service for interacting with voice API endpoints
- Handles authentication token management
- Provides methods for all voice operations:
  - `createChannel()` - Create new voice channel
  - `listChannels()` - Get all available channels
  - `joinChannel()` - Join a channel and get connection token
  - `deleteChannel()` - Delete a channel
  - `isEnabled()` - Check if voice service is available

**2. React Hooks** (`frontend/src/hooks/useVoice.ts`)
- `useVoiceServiceStatus()` - Check if voice service is enabled
- `useVoiceChannels()` - Manage voice channels (list, create, delete)
- `useJoinVoiceChannel()` - Join a channel and get connection token

#### Usage Example

```typescript
import { useVoiceChannels } from './hooks/useVoice';

function VoiceChannelList() {
  const { channels, loading, error, createChannel } = useVoiceChannels();

  const handleCreateChannel = async () => {
    const newChannel = await createChannel({
      name: "Study Group",
      description: "CS101 study session",
      max_users: 10,
      is_temporary: true
    });
    
    if (newChannel) {
      console.log("Channel created:", newChannel);
    }
  };

  if (loading) return <div>Loading...</div>;
  if (error) return <div>Error: {error}</div>;

  return (
    <div>
      <button onClick={handleCreateChannel}>Create Channel</button>
      <ul>
        {channels.map(channel => (
          <li key={channel.channel_id}>{channel.name}</li>
        ))}
      </ul>
    </div>
  );
}
```

#### Environment Variables

The following environment variables can be used to configure the voice service:

```bash
# Enable voice service
export SOHBET_VOICE_ENABLED=true

# Murmur server configuration
export SOHBET_MURMUR_HOST=localhost
export SOHBET_MURMUR_PORT=64738
export SOHBET_MURMUR_ADMIN_PASSWORD=your_secure_password

# Token configuration
export SOHBET_VOICE_TOKEN_EXPIRY=300  # 5 minutes

# Channel configuration
export SOHBET_VOICE_MAX_USERS=25

# Recording (optional)
export SOHBET_VOICE_ENABLE_RECORDING=false
```



---

## Progress Log

### 2025-10-23
- Initial architecture design completed
- Security model defined
- API endpoints specified
- Database schema designed
- Documentation structure created
- Foundation for voice service interface established
- Implemented VoiceChannel model with JSON serialization
- Implemented VoiceConfig for managing voice service configuration
- Created VoiceService interface with stub implementation for testing
- Added comprehensive unit tests for voice services
- Created TypeScript/React frontend service client
- Created React hooks for voice channel management
- All tests passing (5/5 tests including voice service tests)

---

## References

- [Mumble Project](https://www.mumble.info/)
- [Mumble Protocol Documentation](https://mumble-protocol.readthedocs.io/)
- [Murmur Administration Guide](https://wiki.mumble.info/wiki/Murmurguide)
