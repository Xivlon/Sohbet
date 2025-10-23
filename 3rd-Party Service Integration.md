# 3rd-Party Service Integration

## Overview

This document describes the integration of third-party services into Sohbet, with a focus on maintaining security while enabling future features like voice and video group calling.

## Murmur (Mumble Server) Integration

### What is Murmur?

Murmur is the server component of Mumble, an open-source, low-latency, high-quality voice chat application. It provides:
- **Low-latency voice communication**: Essential for real-time conversations
- **Encrypted communications**: All voice traffic is encrypted
- **Permission-based access control**: Fine-grained channel and user permissions
- **Positional audio support**: For gaming and spatial applications
- **Text messaging**: In addition to voice
- **Recording capabilities**: For lectures or important meetings

### Integration Architecture

The Murmur integration is designed with the following principles:

1. **Separation of Concerns**: Voice/audio services are separated from core social media functionality
2. **Security First**: All connections are authenticated through the existing Sohbet authentication system
3. **Future-Proof Design**: Clean interfaces allow easy extension for mobile and desktop clients
4. **Optional Service**: Murmur integration is optional and doesn't affect core functionality

### Architecture Components

```
┌─────────────────────────────────────────────────────────────┐
│                    Sohbet Application                        │
├─────────────────────────────────────────────────────────────┤
│  Frontend (React)                                            │
│  ├── Voice Call UI Components                               │
│  ├── Voice Service Client                                   │
│  └── WebRTC Integration (Future)                            │
├─────────────────────────────────────────────────────────────┤
│  Backend (C++)                                               │
│  ├── Voice Service Interface                                │
│  │   ├── Voice Channel Management                           │
│  │   ├── User Authentication Bridge                         │
│  │   └── Connection Token Generation                        │
│  ├── Murmur Connection Manager                              │
│  │   ├── Configuration Management                           │
│  │   ├── Server Communication                               │
│  │   └── Health Monitoring                                  │
│  └── REST API Endpoints                                     │
│      ├── POST /api/voice/channels (Create voice channel)    │
│      ├── GET /api/voice/channels (List channels)            │
│      ├── POST /api/voice/join (Get connection token)        │
│      └── DELETE /api/voice/channels/:id (Delete channel)    │
├─────────────────────────────────────────────────────────────┤
│  External Services                                           │
│  └── Murmur Server (Optional)                               │
│      ├── Voice/Audio Processing                             │
│      ├── Channel Management                                 │
│      └── Client Connection Handling                         │
└─────────────────────────────────────────────────────────────┘
```

### Security Considerations

#### Authentication Flow

1. User authenticates with Sohbet backend (existing JWT authentication)
2. User requests to join a voice channel
3. Backend validates user permissions and generates a temporary Murmur connection token
4. Token is time-limited (e.g., 5 minutes) and single-use
5. User connects to Murmur server using the token
6. Murmur validates token with Sohbet backend
7. Connection is established or denied based on validation

#### Security Features

- **Token-based authentication**: Prevents unauthorized access to voice channels
- **Time-limited tokens**: Reduces attack window
- **Permission validation**: Users can only join channels they have access to
- **Encrypted communications**: All voice traffic uses TLS/SSL
- **Audit logging**: All voice channel access is logged
- **Rate limiting**: Prevents abuse of token generation

### Implementation Phases

#### Phase 1: Foundation (Current)
- [x] Document integration architecture
- [x] Define security model
- [x] Create voice service interface (C++ header files)
- [x] Implement configuration structure
- [x] Add voice channel data model
- [x] Create frontend TypeScript service client
- [x] Create React hooks for voice integration

#### Phase 2: Core Integration
- [ ] Implement Murmur connection manager
- [ ] Create REST API endpoints for voice services
- [ ] Add token generation and validation
- [ ] Implement basic channel management

#### Phase 3: Frontend Integration
- [ ] Create voice service client (TypeScript)
- [ ] Build UI components for voice channels
- [ ] Implement connection handling
- [ ] Add error handling and reconnection logic

#### Phase 4: Advanced Features
- [ ] WebRTC integration for browser-based calling
- [ ] Mobile client support (iOS/Android)
- [ ] Desktop client integration
- [ ] Screen sharing capabilities
- [ ] Recording and playback features

### Configuration

The Murmur integration requires the following configuration options:

```ini
[voice]
enabled=true
murmur_host=localhost
murmur_port=64738
murmur_admin_password=<secure_password>
token_expiry_seconds=300
max_users_per_channel=25
enable_recording=false
```

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
POST /api/voice/join
Authorization: Bearer <jwt_token>
Content-Type: application/json

{
  "channel_id": 123
}

Response: 200 OK
{
  "connection_token": "eyJ...",
  "murmur_host": "voice.sohbet.app",
  "murmur_port": 64738,
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

1. **WebRTC Integration**: Browser-based calling without Murmur client
2. **Mobile Support**: Native iOS and Android apps with voice support
3. **Video Support**: Extension to include video conferencing
4. **Screen Sharing**: Desktop sharing for presentations
5. **Recording**: Voice channel recording with consent
6. **Transcription**: Automatic transcription of voice conversations
7. **Spatial Audio**: 3D audio for virtual spaces
8. **Noise Suppression**: AI-powered noise cancellation

### Dependencies

The Murmur integration will require:
- **Murmur Server**: Separate installation and configuration
- **ICE (Internet Communications Engine)**: For RPC communication with Murmur (optional)
- **OpenSSL**: For token generation and encryption (already present)
- **SQLite**: For storing channel and token data (already present)

### Deployment Considerations

1. **Murmur Server Setup**: Requires separate server instance
2. **Network Configuration**: Ports 64738 (TCP/UDP) must be accessible
3. **SSL Certificates**: Required for encrypted communications
4. **Resource Planning**: Voice server requires adequate CPU and bandwidth
5. **Monitoring**: Health checks and performance monitoring needed

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
