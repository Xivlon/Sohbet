# WebRTC Developer Quick Reference

**Quick access guide for developers working with Sohbet's WebRTC P2P system**

---

## 🚀 Quick Start

### Running Locally

```bash
# 1. Set up environment
cp .env.example .env
# Edit .env with your DATABASE_URL and SOHBET_JWT_SECRET

# 2. Build backend
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Start backend
./sohbet
# Backend starts on http://0.0.0.0:8080 (HTTP) and ws://0.0.0.0:8081 (WebSocket)

# 4. Install frontend deps
cd ../frontend
npm install

# 5. Start frontend  
npm run dev
# Frontend starts on http://localhost:3000
```

---

## 📡 WebSocket Connection

### Backend (C++)
**Location:** `src/server/websocket_server.cpp`

```cpp
// Initialize WebSocket server (port 8081)
WebSocketServer ws_server(8081);

// Register message handler
ws_server.registerHandler("voice:join", [](int user_id, const WebSocketMessage& msg) {
    // Handle join message
});

// Send message to user
ws_server.sendToUser(user_id, WebSocketMessage("voice:user-joined", payload));

// Broadcast to all
ws_server.broadcast(WebSocketMessage("voice:participants", payload));
```

### Frontend (TypeScript)
**Location:** `frontend/app/lib/websocket-service.ts`

```typescript
import { websocketService } from '@/app/lib/websocket-service';

// Connect with JWT token
await websocketService.connect(jwtToken);

// Send message
websocketService.send('voice:join', { channel_id: 1 });

// Register handler
websocketService.on('voice:user-joined', (message) => {
    console.log('User joined:', message.payload);
});

// Check connection
if (websocketService.isConnected()) {
    // Connected
}
```

---

## 🎙️ WebRTC P2P Connection

### Frontend (TypeScript)
**Location:** `frontend/app/lib/webrtc-service.ts`

```typescript
import { webrtcService } from '@/app/lib/webrtc-service';

// Join voice channel
await webrtcService.joinChannel(channelId, userId, audioOnly = true);

// Get local stream
const localStream = webrtcService.getLocalStream();

// Toggle mute
const isMuted = webrtcService.toggleMute();

// Toggle video
const isVideoOn = await webrtcService.toggleVideo();

// Set participant volume (0-1)
webrtcService.setParticipantVolume(userId, 0.5);

// Leave channel
webrtcService.leaveChannel();

// Listen for participants
webrtcService.onParticipantUpdate((participants) => {
    participants.forEach(p => {
        console.log(p.username, p.isMuted, p.isSpeaking);
    });
});

// Listen for remote streams
webrtcService.onRemoteStream((userId, stream) => {
    // Attach to <audio> or <video> element
    audioElement.srcObject = stream;
});

// Listen for connection quality
webrtcService.onConnectionQuality((quality) => {
    // 'good' | 'medium' | 'poor'
});
```

---

## 🔌 WebSocket Message Types

### Voice Channel Messages

| Type | Direction | Payload | Description |
|------|-----------|---------|-------------|
| `voice:join` | Client → Server | `{ channel_id: number }` | Join a voice channel |
| `voice:leave` | Client → Server | `{ channel_id: number }` | Leave a voice channel |
| `voice:user-joined` | Server → All | `{ user_id, username, channel_id }` | Notify user joined |
| `voice:user-left` | Server → All | `{ user_id, channel_id }` | Notify user left |
| `voice:participants` | Server → Client | `{ participants: [...] }` | List of current participants |
| `voice:offer` | Client ↔ Client | `{ target_user_id, from_user_id, offer }` | WebRTC offer |
| `voice:answer` | Client ↔ Client | `{ target_user_id, from_user_id, answer }` | WebRTC answer |
| `voice:ice-candidate` | Client ↔ Client | `{ target_user_id, from_user_id, candidate }` | ICE candidate |
| `voice:mute` | Client → Server | `{ channel_id, muted: boolean }` | Toggle mute status |
| `voice:video-toggle` | Client → Server | `{ channel_id, video_enabled: boolean }` | Toggle video |
| `voice:user-muted` | Server → All | `{ user_id, muted: boolean }` | Notify mute status |
| `voice:user-video-toggled` | Server → All | `{ user_id, video_enabled: boolean }` | Notify video status |

---

## 🎯 Voice Channel API

**Base URL:** `http://0.0.0.0:8080/api/voice`

### Create Channel
```http
POST /api/voice/channels
Authorization: Bearer <JWT_TOKEN>
Content-Type: application/json

{
    "name": "Study Room",
    "visibility": "public"
}

Response:
{
    "id": 1,
    "name": "Study Room",
    "visibility": "public",
    "creator_id": 123,
    "created_at": "2025-11-15T10:00:00Z"
}
```

### List Channels
```http
GET /api/voice/channels?visibility=public
Authorization: Bearer <JWT_TOKEN>

Response:
{
    "channels": [
        {
            "id": 1,
            "name": "Study Room",
            "visibility": "public",
            "active_users": 3
        }
    ]
}
```

### Get Channel
```http
GET /api/voice/channels/:id
Authorization: Bearer <JWT_TOKEN>

Response:
{
    "id": 1,
    "name": "Study Room",
    "visibility": "public",
    "creator_id": 123,
    "participants": [
        { "user_id": 123, "username": "alice" },
        { "user_id": 456, "username": "bob" }
    ]
}
```

### Join Channel
```http
POST /api/voice/channels/:id/join
Authorization: Bearer <JWT_TOKEN>

Response:
{
    "success": true,
    "session_id": 789
}
```

### Leave Channel
```http
POST /api/voice/channels/:id/leave
Authorization: Bearer <JWT_TOKEN>

Response:
{
    "success": true
}
```

---

## 🔧 Configuration

### Environment Variables

```bash
# Backend
SOHBET_JWT_SECRET=<secret>          # JWT signing secret
DATABASE_URL=postgresql://...        # PostgreSQL connection
HTTP_PORT=8080                       # HTTP API port
WS_PORT=8081                        # WebSocket port

# Frontend
NEXT_PUBLIC_API_URL=http://localhost:8080    # Backend API URL
NEXT_PUBLIC_WS_URL=ws://localhost:8081       # WebSocket URL
```

### ICE Servers (Frontend)

**Location:** `frontend/app/lib/webrtc-service.ts`

```typescript
const DEFAULT_ICE_SERVERS: RTCIceServer[] = [
    // STUN servers
    { urls: 'stun:stun.l.google.com:19302' },
    { urls: 'stun:stun1.l.google.com:19302' },
    
    // TURN servers (for NAT traversal)
    {
        urls: 'turn:global.turn.twilio.com:3478',
        username: 'your-username',
        credential: 'your-credential'
    }
];
```

---

## 🐛 Debugging

### Backend Logs
```bash
# WebSocket connection logs
grep "WebSocket" /path/to/logs

# Voice channel logs
grep "Voice\|voice" /path/to/logs
```

### Frontend Console
```javascript
// Enable verbose WebSocket logging
localStorage.setItem('debug', 'websocket:*');

// Enable WebRTC logging
localStorage.setItem('debug', 'webrtc:*');
```

### Chrome WebRTC Internals
Open `chrome://webrtc-internals` to see:
- Active peer connections
- ICE candidate pairs
- Connection stats
- Media tracks
- Packet loss, jitter, bitrate

### Test WebSocket Connection
```bash
# Use websocat or wscat
npm install -g wscat
wscat -c "ws://localhost:8081/?token=YOUR_JWT_TOKEN"

# Should see: Connection upgraded
# Send: {"type":"voice:join","payload":{"channel_id":1}}
```

---

## 🧪 Testing

### Unit Tests
```bash
cd build

# All tests
ctest

# Specific test
./test_websocket_server
./test_voice_service
```

### Integration Test
```bash
# Use the provided integration test script
chmod +x /tmp/test_webrtc_integration.sh
/tmp/test_webrtc_integration.sh
```

### Manual Testing Checklist
1. [ ] Create user account
2. [ ] Login and get JWT token
3. [ ] Create voice channel
4. [ ] Join voice channel in first tab
5. [ ] Join same channel in second tab (different browser/incognito)
6. [ ] Verify audio in both directions
7. [ ] Test mute/unmute
8. [ ] Test video toggle
9. [ ] Leave channel
10. [ ] Verify cleanup (no memory leaks)

---

## 🔍 Common Issues

### WebSocket Connection Refused
**Problem:** Frontend can't connect to WebSocket  
**Solutions:**
- Verify backend is running
- Check WS_PORT (default 8081)
- Ensure firewall allows port
- Use correct protocol (ws:// vs wss://)

### WebRTC Connection Failed
**Problem:** No audio after joining  
**Solutions:**
- Check browser console for errors
- Grant microphone permissions
- Verify ICE servers are reachable
- Check NAT/firewall settings
- Use TURN server for restrictive NATs

### No Remote Audio
**Problem:** Can't hear other participants  
**Solutions:**
- Check peer connection state
- Verify remote stream is attached to audio element
- Check browser audio output
- Verify remote user isn't muted
- Check connection quality

---

## 📚 Key Files

### Backend
```
src/server/websocket_server.cpp        # WebSocket server
src/voice/voice_service.cpp            # Voice channel service
src/repositories/voice_channel_repository.cpp  # DB operations
include/server/websocket_server.h      # WebSocket interfaces
include/models/voice_channel.h         # Voice channel model
```

### Frontend
```
frontend/app/lib/websocket-service.ts  # WebSocket client
frontend/app/lib/webrtc-service.ts     # WebRTC P2P
frontend/app/lib/voice-service.ts      # Voice channel API
frontend/app/components/khave.tsx      # Voice UI component
```

### Documentation
```
docs/WEBRTC_VALIDATION_REPORT.md       # Complete validation
docs/WEBRTC_PRODUCTION_CHECKLIST.md    # Deployment guide
docs/WEBSOCKET_INFRASTRUCTURE.md       # WebSocket details
docs/VOICE_INTEGRATION.md              # Voice integration
```

---

## 🎯 Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        Frontend (React)                      │
│  ┌────────────────┐  ┌──────────────┐  ┌─────────────────┐ │
│  │ Voice UI       │  │ WebSocket    │  │ WebRTC Service  │ │
│  │ (khave.tsx)    │  │ Service      │  │                 │ │
│  └────────┬───────┘  └──────┬───────┘  └────────┬────────┘ │
│           │                  │                    │          │
└───────────┼──────────────────┼────────────────────┼──────────┘
            │                  │                    │
            │    HTTP API      │  WebSocket (8081)  │  P2P (WebRTC)
            │    (8080)        │                    │
┌───────────┼──────────────────┼────────────────────┼──────────┐
│           │                  │                    │          │
│  ┌────────▼───────┐  ┌──────▼───────┐            │          │
│  │ Voice Channel  │  │ WebSocket    │            │          │
│  │ API            │  │ Server       │            │          │
│  └────────┬───────┘  └──────┬───────┘            │          │
│           │                  │                    │          │
│           ▼                  ▼                    │          │
│  ┌────────────────────────────────┐              │          │
│  │      PostgreSQL Database       │              │          │
│  └────────────────────────────────┘              │          │
│                                                   │          │
│                      Backend (C++)                │          │
└───────────────────────────────────────────────────┼──────────┘
                                                    │
                          ┌─────────────────────────┼─────────────────────┐
                          │                         │                     │
                     ┌────▼────┐              ┌────▼────┐          ┌────▼────┐
                     │ User A  │◄────P2P─────►│ User B  │◄───P2P──►│ User C  │
                     │ Browser │              │ Browser │          │ Browser │
                     └─────────┘              └─────────┘          └─────────┘
                          ▲                         ▲                     ▲
                          │                         │                     │
                          └────────STUN/TURN────────┴─────────────────────┘
                                (NAT Traversal)
```

---

## 💡 Best Practices

### Frontend
1. **Always cleanup:** Call `leaveChannel()` before unmount
2. **Handle errors:** Wrap WebRTC calls in try-catch
3. **User feedback:** Show connection state to user
4. **Permissions:** Request media permissions early
5. **Browser support:** Check for WebRTC support

### Backend
1. **Validate input:** Always validate channel IDs, user IDs
2. **Clean connections:** Remove disconnected users from tracking
3. **Thread safety:** Use mutexes for shared state
4. **Resource limits:** Set max connections per channel
5. **Error handling:** Don't crash on bad WebSocket frames

### Production
1. **Use WSS:** Always use secure WebSocket in production
2. **TURN server:** Deploy dedicated TURN server
3. **Monitoring:** Track connection success rates
4. **Scaling:** Use SFU for channels with 10+ users
5. **Testing:** Test on real mobile devices

---

## 🆘 Support & Resources

- **Code:** `github.com/Xivlon/Sohbet`
- **Docs:** `/docs` directory
- **Issues:** GitHub Issues
- **WebRTC Spec:** https://www.w3.org/TR/webrtc/
- **WebSocket RFC:** https://tools.ietf.org/html/rfc6455

---

**Happy Coding!** 🚀
