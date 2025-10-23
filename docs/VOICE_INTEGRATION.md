# Voice Integration Guide

This guide explains how to use and extend the voice integration in Sohbet.

## Overview

The voice integration provides a foundation for adding voice and video calling features to Sohbet using Murmur (Mumble server). The current implementation includes:

- **Backend Service Interface**: C++ classes for managing voice channels and tokens
- **Frontend Client**: TypeScript service and React hooks
- **Stub Implementation**: For development without a live Murmur server
- **Security Model**: Token-based authentication with time limits

## Quick Start

### Backend Development

1. **Build the project**:
```bash
mkdir build && cd build
cmake ..
make
```

2. **Run tests**:
```bash
ctest --output-on-failure
```

3. **Using the VoiceService**:
```cpp
#include "voice/voice_service.h"

// Create configuration
VoiceConfig config;
config.enabled = true;
config.murmur_host = "localhost";
config.murmur_port = 64738;

// Create service (stub for testing)
VoiceServiceStub service(config);

// Create a channel
VoiceChannel channel = service.create_channel(
    "Study Group",
    "CS101 meeting",
    user_id,
    10,
    true  // temporary
);

// Generate connection token
VoiceConnectionToken token = service.generate_connection_token(
    user_id,
    channel.id
);
```

### Frontend Development

1. **Import the service and hooks**:
```typescript
import { voiceService } from './services/voiceService';
import { useVoiceChannels, useJoinVoiceChannel } from './hooks/useVoice';
```

2. **Use in a component**:
```typescript
function MyComponent() {
  const { channels, loading, createChannel } = useVoiceChannels();
  const { joinChannel, connectionToken } = useJoinVoiceChannel();

  // Create a channel
  const handleCreate = async () => {
    await createChannel({
      name: "My Channel",
      description: "Test channel",
      max_users: 10
    });
  };

  // Join a channel
  const handleJoin = async (channelId: number) => {
    const token = await joinChannel(channelId);
    console.log("Connection token:", token);
  };

  return (
    // Your UI here
  );
}
```

## Configuration

### Environment Variables

Set these environment variables to configure the voice service:

```bash
# Enable/disable voice service
export SOHBET_VOICE_ENABLED=true

# Murmur server settings
export SOHBET_MURMUR_HOST=localhost
export SOHBET_MURMUR_PORT=64738
export SOHBET_MURMUR_ADMIN_PASSWORD=your_password

# Token expiry in seconds (default: 300 = 5 minutes)
export SOHBET_VOICE_TOKEN_EXPIRY=300

# Maximum users per channel (default: 25)
export SOHBET_VOICE_MAX_USERS=25

# Enable recording (default: false)
export SOHBET_VOICE_ENABLE_RECORDING=false
```

## Architecture

### Backend Components

```
include/voice/
  ├── voice_config.h      # Configuration management
  └── voice_service.h     # Service interface and stub

include/models/
  └── voice_channel.h     # Channel data model

src/voice/
  ├── voice_config.cpp
  └── voice_service.cpp

tests/
  └── test_voice_service.cpp  # Unit tests
```

### Frontend Components

```
frontend/src/
  ├── services/
  │   └── voiceService.ts    # Voice API client
  └── hooks/
      └── useVoice.ts        # React hooks
```

## Extending the Integration

### Adding Real Murmur Integration

To replace the stub with a real Murmur integration:

1. **Create a MurmurService class**:
```cpp
class MurmurService : public VoiceService {
  // Implement actual Murmur communication
  // Use ICE for RPC or native protocol
};
```

2. **Implement the interface methods**:
- `create_channel()` - Create channel on Murmur server
- `delete_channel()` - Remove channel from Murmur
- `generate_connection_token()` - Generate secure tokens
- `validate_token()` - Verify tokens with Murmur

3. **Update database integration**:
- Store channels in database
- Implement token table for validation
- Add access logging

### Adding WebRTC Support

For browser-based calling without Murmur:

1. **Create WebRTCService**:
```typescript
class WebRTCService {
  async createOffer(): Promise<RTCSessionDescription>;
  async createAnswer(): Promise<RTCSessionDescription>;
  async addIceCandidate(candidate: RTCIceCandidate): Promise<void>;
}
```

2. **Add signaling server**:
- WebSocket connections for signaling
- SDP exchange between peers
- ICE candidate exchange

### Mobile Client Support

For mobile apps (iOS/Android):

1. **Use native Mumble libraries**:
- iOS: Mumble iOS SDK
- Android: Mumble Android library

2. **Implement token exchange**:
```swift
// iOS example
func joinChannel(token: String, host: String, port: Int) {
    let connection = MumbleConnection(
        host: host,
        port: port,
        username: username,
        password: token
    )
    connection.connect()
}
```

## Security Considerations

### Token Generation

Currently using simple hash-based tokens (stub). For production:

1. **Use JWT tokens**:
```cpp
// Example with JWT library
std::string generate_jwt_token(int user_id, int channel_id) {
    auto token = jwt::create()
        .set_issuer("sohbet")
        .set_subject(std::to_string(user_id))
        .set_payload_claim("channel_id", jwt::claim(std::to_string(channel_id)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{300})
        .sign(jwt::algorithm::hs256{"your-secret-key"});
    return token;
}
```

2. **Store tokens in database**:
```sql
CREATE TABLE voice_tokens (
    id INTEGER PRIMARY KEY,
    token_hash TEXT UNIQUE,
    user_id INTEGER,
    channel_id INTEGER,
    expires_at TIMESTAMP,
    used BOOLEAN DEFAULT 0
);
```

### Permission Checking

Add permission checks before allowing operations:

```cpp
bool VoiceService::can_join_channel(int user_id, int channel_id) {
    // Check if user has permission to join
    // Check channel capacity
    // Check if channel is private/public
    return true;
}
```

## Testing

### Running Tests

```bash
cd build
ctest -V  # Verbose output
```

### Adding New Tests

Create test files in `tests/` directory:

```cpp
#include "voice/voice_service.h"
#include <cassert>

void test_my_feature() {
    // Your test code
    assert(condition);
}

int main() {
    test_my_feature();
    return 0;
}
```

Add to `CMakeLists.txt`:

```cmake
add_executable(test_my_feature tests/test_my_feature.cpp)
target_link_libraries(test_my_feature sohbet_lib)
add_test(NAME MyFeatureTest COMMAND test_my_feature)
```

## Troubleshooting

### Common Issues

1. **Voice service not enabled**:
   - Check `SOHBET_VOICE_ENABLED` environment variable
   - Verify configuration with `config.is_valid()`

2. **Frontend can't connect**:
   - Check API endpoint paths
   - Verify authentication token is set
   - Check CORS settings on backend

3. **Build errors**:
   - Ensure all dependencies are installed
   - Run `cmake ..` again to regenerate build files
   - Check compiler version (C++17 required)

## Next Steps

See the main [3rd-Party Service Integration.md](../3rd-Party Service Integration.md) document for:
- Complete API documentation
- Security model details
- Database schema
- Deployment considerations
- Future enhancements

## Support

For questions or issues:
1. Check the documentation in `3rd-Party Service Integration.md`
2. Review test files for usage examples
3. Create an issue on GitHub with details
