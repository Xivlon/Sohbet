# WebSocket Infrastructure

## Overview

The Sohbet platform now includes WebSocket infrastructure for real-time messaging features. This enables instant message delivery, typing indicators, and online/offline presence tracking.

## Architecture

### Backend (C++)

The WebSocket server is implemented in C++ and runs on port 8081 alongside the HTTP server (port 8080).

**Key Components:**
- `WebSocketServer` - Manages WebSocket connections and message routing
- `WebSocketConnection` - Represents a single client connection
- `WebSocketMessage` - Standard message format for all WebSocket communications

**Files:**
- `include/server/websocket_server.h` - WebSocket server interface
- `src/server/websocket_server.cpp` - WebSocket server implementation

### Frontend (React/TypeScript)

The frontend uses a service-based architecture for WebSocket management with React hooks for easy integration.

**Key Components:**
- `websocketService` - Singleton service managing the WebSocket connection
- `useWebSocket` - Hook for automatic connection management
- `useChatWebSocket` - Hook for chat-specific WebSocket features
- `useOnlineUsers` - Hook for tracking online/offline status

**Files:**
- `app/lib/websocket-service.ts` - WebSocket client service
- `app/lib/use-websocket.ts` - React hooks for WebSocket
- `app/components/websocket-provider.tsx` - Provider component

## Features

### 1. Real-Time Chat Messages

Messages are delivered instantly through WebSocket connections without polling.

**Backend Handler:**
```cpp
websocket_server_->registerHandler("chat:send", handleChatMessage);
```

**Frontend Usage:**
```typescript
const { sendMessage } = useChatWebSocket(conversationId);
sendMessage("Hello!");
```

### 2. Typing Indicators

Users can see when others are typing in a conversation.

**Backend Handler:**
```cpp
websocket_server_->registerHandler("chat:typing", handleTypingIndicator);
```

**Frontend Usage:**
```typescript
const { sendTyping, typingUsers } = useChatWebSocket(conversationId);

// Send typing indicator
sendTyping();

// Display typing users
{typingUsers.length > 0 && <div>Someone is typing...</div>}
```

### 3. Online/Offline Presence

Track which users are currently online.

**Backend:**
Automatically broadcasts `user:online` and `user:offline` events when users connect/disconnect.

**Frontend Usage:**
```typescript
const { isOnline, onlineUsers } = useOnlineUsers();

{isOnline(userId) ? <GreenDot /> : <GrayDot />}
```

## Message Protocol

All WebSocket messages follow this JSON format:

```json
{
  "type": "message_type",
  "payload": { ... }
}
```

### Message Types

#### Client → Server

**chat:send** - Send a chat message
```json
{
  "type": "chat:send",
  "payload": {
    "conversation_id": 1,
    "content": "Hello!"
  }
}
```

**chat:typing** - Send typing indicator
```json
{
  "type": "chat:typing",
  "payload": {
    "conversation_id": 1
  }
}
```

#### Server → Client

**chat:message** - New message received
```json
{
  "type": "chat:message",
  "payload": {
    "id": 123,
    "conversation_id": 1,
    "sender_id": 2,
    "content": "Hello!",
    "created_at": "2025-10-27T20:00:00Z"
  }
}
```

**chat:typing** - User is typing
```json
{
  "type": "chat:typing",
  "payload": {
    "conversation_id": 1,
    "user_id": 2
  }
}
```

**user:online** - User came online
```json
{
  "type": "user:online",
  "payload": {
    "user_id": 2
  }
}
```

**user:offline** - User went offline
```json
{
  "type": "user:offline",
  "payload": {
    "user_id": 2
  }
}
```

## Authentication

WebSocket connections are authenticated using JWT tokens:

1. Client obtains JWT token from login endpoint
2. Client connects to WebSocket with token as query parameter:
   ```
   ws://localhost:8081/?token=eyJhbGciOiJIUzI1NiIs...
   ```
3. Server validates token and extracts user_id
4. Connection is established and associated with user_id

## Connection Management

### Auto-Reconnection

The frontend WebSocket service implements automatic reconnection with exponential backoff:

- Initial delay: 1 second
- Max attempts: 5
- Backoff: delay × 2^(attempts-1)

```typescript
// Automatically handled by the service
const { connected } = useWebSocket(token);
```

### Connection Status

Display connection status to users:

```typescript
import { ConnectionStatus } from '@/components/websocket-provider';

<ConnectionStatus className="absolute top-4 right-4" />
```

## Usage Guide

### 1. Setup WebSocket Provider

Wrap your app with the WebSocket provider:

```typescript
// app/layout.tsx
import { WebSocketProvider } from '@/components/websocket-provider';

export default function RootLayout({ children }) {
  const token = getAuthToken(); // Your auth token
  
  return (
    <WebSocketProvider token={token}>
      {children}
    </WebSocketProvider>
  );
}
```

### 2. Use in Chat Component

```typescript
'use client';

import { useChatWebSocket, useWebSocketMessage } from '@/lib/use-websocket';

export function ChatWindow({ conversationId }) {
  const { sendMessage, sendTyping, typingUsers } = useChatWebSocket(conversationId);
  const [messages, setMessages] = useState([]);

  // Listen for new messages
  useWebSocketMessage('chat:message', (message) => {
    if (message.payload.conversation_id === conversationId) {
      setMessages(prev => [...prev, message.payload]);
    }
  });

  const handleSend = () => {
    // Try WebSocket first, fallback to REST API
    const sent = sendMessage(messageContent);
    if (!sent) {
      // Fallback to REST API
      await fetch('/api/conversations/...');
    }
  };

  return (
    <div>
      {/* Display messages */}
      {typingUsers.length > 0 && <div>Someone is typing...</div>}
      
      <input 
        onChange={() => sendTyping()}
        onKeyPress={handleSend}
      />
    </div>
  );
}
```

## Testing

### Backend Tests

```bash
cd build
./test_websocket_server
```

### Manual Testing

1. Start the server:
```bash
cd build
./sohbet
```

2. Connect with a WebSocket client (e.g., browser console):
```javascript
const token = 'your_jwt_token_here';
const ws = new WebSocket(`ws://localhost:8081/?token=${token}`);

ws.onmessage = (event) => {
  console.log('Received:', event.data);
};

// Send a message
ws.send(JSON.stringify({
  type: 'chat:send',
  payload: {
    conversation_id: 1,
    content: 'Test message'
  }
}));
```

## Performance Considerations

### Backend
- Connection pooling: Multiple connections per user supported
- Thread-based client handling: Each client runs in a separate thread
- Efficient message routing: O(1) lookup for user connections

### Frontend
- Automatic reconnection prevents permanent disconnections
- Message handlers are efficiently managed with Sets
- Connection status listeners allow UI updates

## Security

### Authentication
- All connections must provide valid JWT token
- Token verified before establishing connection
- Invalid tokens immediately rejected

### Authorization
- Users can only send messages to conversations they're part of
- Server validates conversation membership before forwarding messages
- No cross-user message leakage

### Data Validation
- All incoming messages validated before processing
- Malformed JSON safely handled
- Type checking on message payloads

## Troubleshooting

### Connection Fails
1. Check that WebSocket server is running on port 8081
2. Verify JWT token is valid
3. Check browser console for errors
4. Ensure firewall allows WebSocket connections

### Messages Not Received
1. Verify WebSocket connection is established
2. Check message handler is registered
3. Ensure conversation_id is correct
4. Check server logs for errors

### Reconnection Issues
1. Check max reconnection attempts not exceeded
2. Verify token is still valid
3. Check network connectivity
4. Review browser console for reconnection attempts

## Future Enhancements

- [ ] Message read receipts via WebSocket
- [ ] Voice call signaling
- [ ] Video call support
- [ ] Group chat support
- [ ] File transfer notifications
- [ ] Screen sharing coordination
- [ ] Custom message types for plugins

## API Reference

See the following files for detailed API documentation:
- `include/server/websocket_server.h` - Backend WebSocket API
- `app/lib/websocket-service.ts` - Frontend WebSocket service
- `app/lib/use-websocket.ts` - React hooks documentation
