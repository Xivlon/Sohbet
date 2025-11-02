# Frontend-Backend Integration Quick Reference

## Quick Status Check

### Backend Health
```bash
curl https://sohbet-uezxqq.fly.dev/api/status
```

Expected response:
```json
{
  "status": "ok",
  "version": "0.3.0-academic",
  "features": ["authentication", "posts", "groups", "chat", "voice"]
}
```

### Frontend Build
```bash
cd frontend
npm run build
```

Should complete without TypeScript errors.

## Environment Variables

### Required Variables
```env
# Production
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081

# Local Development
NEXT_PUBLIC_API_URL=http://localhost:8080
NEXT_PUBLIC_WS_URL=ws://localhost:8081
```

### Verification
Check variables are loaded:
```javascript
// In browser console
console.log('API URL:', process.env.NEXT_PUBLIC_API_URL);
```

## Common Error Codes

### 401 Unauthorized
**Causes**:
- Missing authentication token
- Invalid token
- Expired token

**Fix**:
1. Clear localStorage: `localStorage.clear()`
2. Login again
3. Check Authorization header includes `Bearer <token>`

### 403 Forbidden
**Causes**:
- Insufficient permissions
- Wrong user role

**Fix**:
1. Verify user role in localStorage
2. Check backend permission configuration
3. Ensure endpoint allows user's role

### 404 Not Found
**Causes**:
- Incorrect API endpoint
- Backend route doesn't exist
- Typo in URL

**Fix**:
1. Verify endpoint exists in backend
2. Check for typos in API calls
3. Ensure `NEXT_PUBLIC_API_URL` is correct

### Network Error
**Causes**:
- Backend not running
- CORS misconfiguration
- Firewall blocking

**Fix**:
1. Check backend is running: `curl https://sohbet-uezxqq.fly.dev/api/status`
2. Verify CORS_ORIGIN on backend matches frontend URL
3. Check browser console for specific error

## API Endpoints Reference

### Authentication
```
POST /api/login
POST /api/users (register)
GET  /api/users/demo
GET  /api/users/:id
PUT  /api/users/:id
```

### Posts
```
GET    /api/posts?limit=50&offset=0
POST   /api/posts
DELETE /api/posts/:id
POST   /api/posts/:id/react
DELETE /api/posts/:id/react
```

### Comments
```
GET    /api/posts/:id/comments
POST   /api/posts/:id/comments
POST   /api/comments/:id/reply
DELETE /api/comments/:id
```

### Groups
```
GET    /api/groups?limit=50&offset=0
POST   /api/groups
POST   /api/groups/:id/members
DELETE /api/groups/:id/members/:userId
```

### Organizations
```
GET  /api/organizations?limit=50&offset=0
POST /api/organizations
```

### Friendships
```
GET    /api/friendships?status=pending
PUT    /api/friendships/:id/accept
PUT    /api/friendships/:id/reject
DELETE /api/friendships/:id
GET    /api/users/:id/friends
```

### Conversations/Messages
```
GET  /api/conversations
POST /api/conversations
GET  /api/conversations/:id/messages?limit=50&offset=0
POST /api/conversations/:id/messages
```

### Voice Channels
```
GET    /api/voice/channels?channel_type=public
POST   /api/voice/channels
GET    /api/voice/channels/:id
POST   /api/voice/channels/:id/join
DELETE /api/voice/channels/:id/leave
DELETE /api/voice/channels/:id
```

### Media
```
POST /api/media/upload
GET  /api/users/:id/media
GET  /api/media/file/:storageKey
```

## WebSocket Events

### Client → Server
```javascript
{
  type: 'chat:send',
  payload: {
    conversation_id: 1,
    content: 'Hello'
  }
}

{
  type: 'chat:typing',
  payload: {
    conversation_id: 1,
    user_id: 1,
    username: 'john'
  }
}
```

### Server → Client
```javascript
{
  type: 'chat:message',
  payload: {
    id: 123,
    conversation_id: 1,
    sender_id: 2,
    content: 'Hi there',
    created_at: '2025-11-02T10:00:00Z'
  }
}

{
  type: 'user:online',
  payload: {
    user_id: 2,
    username: 'jane'
  }
}

{
  type: 'user:offline',
  payload: {
    user_id: 2,
    username: 'jane'
  }
}
```

## TypeScript Interfaces

All API responses are fully typed. Import from `api-client.ts`:

```typescript
import { 
  User,
  Post,
  Comment,
  Group,
  Organization,
  Conversation,
  Message,
  Friendship,
  VoiceChannel,
  apiClient 
} from '@/app/lib/api-client';

// Example usage
const response = await apiClient.getPosts(50, 0);
if (response.data) {
  const posts: Post[] = response.data.posts;
  // Full type safety and IntelliSense
}
```

## Debug Mode

Enable debug logging:
```env
NEXT_PUBLIC_DEBUG_API=true
```

Or set in code:
```javascript
// In browser console
localStorage.setItem('debugEnabled', 'true');
```

## Testing Checklist

### Before Deployment
- [ ] `npm run build` succeeds
- [ ] `npm run lint` shows no errors
- [ ] Environment variables configured
- [ ] `.gitignore` excludes `.env.local`

### After Deployment
- [ ] Homepage loads
- [ ] Can register new account
- [ ] Can login
- [ ] Token stored in localStorage
- [ ] API calls go to correct backend
- [ ] WebSocket connects successfully
- [ ] Can create/view posts
- [ ] Can send messages
- [ ] No console errors

## Troubleshooting Commands

### Clear Browser Data
```javascript
// In browser console
localStorage.clear();
sessionStorage.clear();
location.reload();
```

### Test API Directly
```bash
# Test authentication
curl -X POST https://sohbet-uezxqq.fly.dev/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"demo_student","password":"demo123"}'

# Test with token
curl https://sohbet-uezxqq.fly.dev/api/posts \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

### Test WebSocket
```javascript
// In browser console
const ws = new WebSocket('wss://sohbet-uezxqq.fly.dev:8081');
ws.onopen = () => console.log('✅ Connected');
ws.onerror = (e) => console.error('❌ Error:', e);
ws.onclose = () => console.log('🔌 Disconnected');

// Authenticate
ws.send(JSON.stringify({
  type: 'auth',
  token: 'YOUR_TOKEN_HERE'
}));
```

## Support Resources

- **Full Deployment Guide**: `FRONTEND_DEPLOYMENT.md`
- **Backend Deployment**: `docs/DEPLOYMENT_GUIDE.md`
- **WebSocket Issues**: `docs/WEBSOCKET_HANDSHAKE_FIX.md`
- **API Documentation**: `README.md` (API Reference section)

---

**Quick Links**:
- Backend: https://sohbet-uezxqq.fly.dev
- Frontend: https://sohbet-seven.vercel.app
- Repository: https://github.com/Xivlon/Sohbet
