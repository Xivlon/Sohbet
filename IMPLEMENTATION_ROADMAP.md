# Sohbet Social Features - Implementation Roadmap

## Overview
This document provides a complete roadmap for implementing advanced social features in the Sohbet academic platform. The database schema has already been created with 17 new tables supporting all features.

---

## 🗄️ Database Status: ✅ COMPLETE

All database tables have been created and seeded:
- **Role System**: 3 roles (Student, Professor, Admin) with 13 permissions
- **User Distribution**: 27 users (24 Students, 3 Professors)
- **Tables Created**: 17 new tables for all social features

---

## 📊 Implementation Phases

### **PHASE 1: Foundation Features** (Recommended Priority)
Core features that other features depend on.

#### 1.1 Profile Photos & Media Upload
**Complexity**: Medium | **Time**: 2-3 days

**Backend Tasks:**
1. Install Replit Object Storage blueprint
2. Create `MediaRepository` class for user_media table
3. Create API endpoints:
   - `POST /api/users/:id/avatar` - Upload profile photo
   - `GET /api/users/:id/avatar` - Get avatar URL
   - `DELETE /api/users/:id/avatar` - Remove avatar
4. Add file validation (size limit: 5MB, types: jpg, png, webp)
5. Generate signed URLs for secure access

**Frontend Tasks:**
1. Create `AvatarUpload` component with drag-drop interface
2. Add image preview and crop functionality
3. Update Profile page to show avatar with upload button
4. Add avatar display to all user references (sidebar, posts, etc.)
5. Implement loading states and error handling

**API Specification:**
```http
POST /api/users/:id/avatar
Content-Type: multipart/form-data
Authorization: Bearer {token}

Body: { file: [binary image data] }

Response 200:
{
  "avatar_url": "https://storage.replit.com/...",
  "user": { ...updated user object }
}
```

**Files to Create/Modify:**
- `src/repositories/media_repository.cpp`
- `include/repositories/media_repository.h`
- `frontend/app/components/avatar-upload.tsx`
- `frontend/app/components/profile.tsx` (update)

---

#### 1.2 Role-Based Access Control (RBAC)
**Complexity**: Medium | **Time**: 1-2 days

**Backend Tasks:**
1. Create `RoleRepository` and `PermissionRepository`
2. Add middleware for permission checking
3. Update JWT token to include user role
4. Create helper functions: `hasPermission()`, `isRole()`

**Frontend Tasks:**
1. Update auth context to include user role
2. Add permission checking hooks: `usePermission()`, `useRole()`
3. Conditionally render UI based on permissions
4. Add "Upgrade to Professor" workflow (if applicable)

**Permission Gates:**
- `create_group` → Professor only
- `create_public_post` → Professor only
- `manage_users` → Admin only
- `delete_any_post` → Admin only

---

### **PHASE 2: Social Features** (High Value)
Features that make the platform social and engaging.

#### 2.1 Friend Request System
**Complexity**: Medium | **Time**: 2-3 days

**Backend Tasks:**
1. Create `FriendshipRepository` class
2. Create API endpoints:
   - `POST /api/friendships` - Send friend request
   - `GET /api/friendships` - List friend requests (incoming/outgoing/accepted)
   - `PUT /api/friendships/:id/accept` - Accept request
   - `PUT /api/friendships/:id/reject` - Reject request
   - `DELETE /api/friendships/:id` - Unfriend/cancel request
3. Add notifications for new friend requests

**Frontend Tasks:**
1. Create `FriendRequests` component
2. Create `FriendsList` component
3. Add "Add Friend" button to user profiles
4. Add friend request notifications in header
5. Create friends management page

**Database Query Examples:**
```sql
-- Get all friends for user
SELECT u.* FROM users u
JOIN friendships f ON (f.requester_id = u.id OR f.addressee_id = u.id)
WHERE (f.requester_id = ? OR f.addressee_id = ?)
AND f.status = 'accepted'
AND u.id != ?;

-- Get pending incoming requests
SELECT u.* FROM users u
JOIN friendships f ON f.requester_id = u.id
WHERE f.addressee_id = ? AND f.status = 'pending';
```

---

#### 2.2 Posts System
**Complexity**: Medium-High | **Time**: 3-4 days

**Backend Tasks:**
1. Create `PostRepository` class
2. Create API endpoints:
   - `POST /api/posts` - Create post (from profile only)
   - `GET /api/posts` - Feed (friends' posts + own posts)
   - `GET /api/users/:id/posts` - User's posts
   - `PUT /api/posts/:id` - Edit post
   - `DELETE /api/posts/:id` - Delete post
   - `POST /api/posts/:id/react` - React to post
3. Implement visibility rules (public/friends/private)

**Frontend Tasks:**
1. Create `PostComposer` component (on profile page only)
2. Create `PostCard` component
3. Create `Feed` component with infinite scroll
4. Add reaction buttons (like, love, insightful)
5. Implement real-time updates for new posts

**Visibility Logic:**
```typescript
// Backend pseudo-code
function canViewPost(post, viewer) {
  if (post.visibility === 'public') return true;
  if (post.author_id === viewer.id) return true;
  if (post.visibility === 'friends') {
    return areFriends(post.author_id, viewer.id);
  }
  if (post.visibility === 'group' && post.group_id) {
    return isMemberOfGroup(viewer.id, post.group_id);
  }
  return false;
}
```

---

#### 2.3 Comments & Replies
**Complexity**: Medium-High | **Time**: 2-3 days

**Backend Tasks:**
1. Create `CommentRepository` class
2. Create API endpoints:
   - `POST /api/posts/:id/comments` - Add comment
   - `GET /api/posts/:id/comments` - Get comments (with threading)
   - `POST /api/comments/:id/reply` - Reply to comment
   - `PUT /api/comments/:id` - Edit comment
   - `DELETE /api/comments/:id` - Delete comment

**Frontend Tasks:**
1. Create `CommentThread` component with nested rendering
2. Create `CommentForm` component
3. Add "Reply" functionality with indentation
4. Implement optimistic UI updates
5. Add real-time comment notifications

**Threading Strategy:**
```sql
-- Recursive CTE for nested comments
WITH RECURSIVE comment_tree AS (
  SELECT *, 0 as depth FROM comments 
  WHERE post_id = ? AND parent_id IS NULL
  UNION ALL
  SELECT c.*, ct.depth + 1 
  FROM comments c
  JOIN comment_tree ct ON c.parent_id = ct.id
)
SELECT * FROM comment_tree ORDER BY depth, created_at;
```

---

### **PHASE 3: Groups & Organizations** (Professor Features)
Features for academic collaboration and organization management.

#### 3.1 Groups (Professor-Created)
**Complexity**: High | **Time**: 4-5 days

**Backend Tasks:**
1. Create `GroupRepository` and `GroupMemberRepository`
2. Create API endpoints:
   - `POST /api/groups` - Create group (Professor only)
   - `GET /api/groups` - List groups
   - `GET /api/groups/:id` - Group details
   - `PUT /api/groups/:id` - Update group (admin only)
   - `DELETE /api/groups/:id` - Delete group (admin only)
   - `POST /api/groups/:id/members` - Add member
   - `DELETE /api/groups/:id/members/:userId` - Remove member
   - `PUT /api/groups/:id/members/:userId/role` - Change member role
3. Add permission checks for all operations

**Frontend Tasks:**
1. Create `GroupCreator` component (Professor-only)
2. Create `GroupCard` and `GroupsList` components
3. Create `GroupManagement` page with member management
4. Add group posts feed
5. Add "Groups" navigation item (visible to all, create button for Professors only)

---

#### 3.2 Organizations/Clubs
**Complexity**: Medium-High | **Time**: 3-4 days

**Backend Tasks:**
1. Create `OrganizationRepository`
2. Similar endpoints to Groups but for organizations
3. Add logo upload capability
4. Add organization account management

**Frontend Tasks:**
1. Create organization profile pages
2. Add organization post authoring
3. Create organization discovery/directory
4. Add "Manage Organization" page for admins

---

### **PHASE 4: Real-Time Communication**
Advanced features requiring WebSocket infrastructure.

#### 4.1 Real-Time Chat
**Complexity**: Very High | **Time**: 5-7 days

**Backend Tasks:**
1. Set up WebSocket server (using `blueprint:javascript_websocket`)
2. Create `ConversationRepository` and `MessageRepository`
3. Implement WebSocket authentication via JWT
4. Create chat event handlers:
   - `chat:send` - Send message
   - `chat:typing` - Typing indicator
   - `chat:read` - Mark as read
5. Create REST API endpoints for message history
6. Add message delivery/read receipts

**Frontend Tasks:**
1. Set up WebSocket client connection
2. Create `ChatList` component (list of conversations)
3. Create `ChatWindow` component with message bubbles
4. Add typing indicators
5. Add message status indicators (sent/delivered/read)
6. Add real-time notifications for new messages
7. Implement message pagination

**WebSocket Protocol:**
```typescript
// Client → Server
{
  type: 'chat:send',
  payload: {
    conversation_id: number,
    content: string,
    media_url?: string
  }
}

// Server → Client
{
  type: 'chat:message',
  payload: {
    id: number,
    conversation_id: number,
    sender_id: number,
    content: string,
    created_at: string
  }
}
```

---

#### 4.2 Voice Calls & Murmur Integration (Khave)
**Complexity**: Very High | **Time**: 7-10 days

**Backend Tasks:**
1. Research Murmur server setup and API
2. Deploy Murmur server instance
3. Create `VoiceChannelRepository` and `VoiceSessionRepository`
4. Create API endpoints:
   - `POST /api/voice/channels` - Create channel
   - `POST /api/voice/channels/:id/join` - Join channel
   - `DELETE /api/voice/channels/:id/leave` - Leave channel
   - `GET /api/voice/sessions` - Active sessions
5. Implement signaling for WebRTC
6. Create Khave public discussion channels

**Frontend Tasks:**
1. Integrate WebRTC for audio/video
2. Create `VoiceChannel` component
3. Create `Khave` public discussion interface
4. Add voice controls (mute, deafen, volume)
5. Add participant list with speaking indicators
6. Implement screen sharing capability

**Note:** This is the most complex feature. Consider using a third-party service like Daily.co, Twilio, or Agora for faster implementation.

---

## 🔧 Technical Implementation Details

### Backend Architecture

**New C++ Classes to Create:**

```
src/repositories/
├── media_repository.cpp
├── friendship_repository.cpp
├── post_repository.cpp
├── comment_repository.cpp
├── group_repository.cpp
├── organization_repository.cpp
├── conversation_repository.cpp
├── message_repository.cpp
└── voice_repository.cpp

include/repositories/
└── [corresponding .h files]

src/services/
├── permission_service.cpp
├── storage_service.cpp
└── websocket_service.cpp
```

**API Endpoint Summary (50+ new endpoints):**

```
Authentication & Users:
✅ POST /api/login
✅ POST /api/users
✅ GET /api/users
✅ PUT /api/users/:id

Media:
POST /api/users/:id/avatar
DELETE /api/users/:id/avatar
POST /api/media/upload

Friendships:
POST /api/friendships
GET /api/friendships
PUT /api/friendships/:id/accept
PUT /api/friendships/:id/reject
DELETE /api/friendships/:id

Posts:
POST /api/posts
GET /api/posts
GET /api/users/:id/posts
PUT /api/posts/:id
DELETE /api/posts/:id
POST /api/posts/:id/react
DELETE /api/posts/:id/react

Comments:
POST /api/posts/:id/comments
GET /api/posts/:id/comments
POST /api/comments/:id/reply
PUT /api/comments/:id
DELETE /api/comments/:id

Groups:
POST /api/groups
GET /api/groups
GET /api/groups/:id
PUT /api/groups/:id
DELETE /api/groups/:id
POST /api/groups/:id/members
DELETE /api/groups/:id/members/:userId
PUT /api/groups/:id/members/:userId/role

Organizations:
POST /api/organizations
GET /api/organizations
GET /api/organizations/:id
PUT /api/organizations/:id
POST /api/organizations/:id/accounts
DELETE /api/organizations/:id/accounts/:userId

Chat:
GET /api/conversations
GET /api/conversations/:id/messages
POST /api/conversations/:id/messages
PUT /api/messages/:id/read

Voice:
POST /api/voice/channels
GET /api/voice/channels
POST /api/voice/channels/:id/join
DELETE /api/voice/channels/:id/leave
```

---

### Frontend Architecture

**New Components to Create:**

```
frontend/app/components/
├── media/
│   ├── avatar-upload.tsx
│   ├── avatar.tsx
│   └── image-crop.tsx
├── social/
│   ├── friend-request-card.tsx
│   ├── friend-list.tsx
│   └── friend-button.tsx
├── posts/
│   ├── post-composer.tsx
│   ├── post-card.tsx
│   ├── post-feed.tsx
│   └── post-reactions.tsx
├── comments/
│   ├── comment-thread.tsx
│   ├── comment-form.tsx
│   └── comment-card.tsx
├── groups/
│   ├── group-creator.tsx
│   ├── group-card.tsx
│   ├── group-list.tsx
│   └── group-management.tsx
├── organizations/
│   ├── organization-profile.tsx
│   ├── organization-directory.tsx
│   └── organization-manager.tsx
├── chat/
│   ├── chat-list.tsx
│   ├── chat-window.tsx
│   ├── message-bubble.tsx
│   └── typing-indicator.tsx
└── voice/
    ├── voice-channel.tsx
    ├── voice-controls.tsx
    └── khave-room.tsx
```

**New Pages:**

```
frontend/app/
├── friends/
│   └── page.tsx
├── groups/
│   ├── page.tsx
│   └── [id]/page.tsx
├── organizations/
│   ├── page.tsx
│   └── [id]/page.tsx
├── messages/
│   ├── page.tsx
│   └── [conversationId]/page.tsx
└── khave/
    └── page.tsx
```

**State Management:**

Consider using Zustand for global state:

```typescript
// stores/usePostsStore.ts
interface PostsStore {
  posts: Post[];
  addPost: (post: Post) => void;
  updatePost: (id: number, post: Partial<Post>) => void;
  deletePost: (id: number) => void;
}

// stores/useChatStore.ts
interface ChatStore {
  conversations: Conversation[];
  activeConversation: number | null;
  messages: Record<number, Message[]>;
  sendMessage: (conversationId: number, content: string) => void;
}

// stores/useFriendsStore.ts
interface FriendsStore {
  friends: User[];
  pendingRequests: Friendship[];
  sendRequest: (userId: number) => void;
  acceptRequest: (requestId: number) => void;
}
```

---

## 🧪 Testing Strategy

### Unit Tests
- Repository CRUD operations
- Permission checking logic
- Friendship status transitions
- Post visibility rules

### Integration Tests
- Complete friend request flow
- Post creation → comment → reaction flow
- Group creation → member addition → post
- Chat message delivery and read receipts

### E2E Tests
- User registration → profile setup → post → friend → chat
- Professor creates group → adds members → members post
- Organization account creates public post

---

## 📈 Performance Considerations

### Database Optimization
1. Add indexes on frequently queried columns (already added for some)
2. Use pagination for all list endpoints (limit: 20-50 items)
3. Implement caching for user profiles and permissions
4. Use database connection pooling

### Frontend Optimization
1. Lazy load components (React.lazy)
2. Implement virtual scrolling for long lists
3. Optimize image loading (lazy loading, responsive images)
4. Debounce search and typing indicators
5. Use React Query for data caching

### WebSocket Optimization
1. Implement connection pooling
2. Add heartbeat/ping-pong for connection health
3. Batch multiple events when possible
4. Implement exponential backoff for reconnection

---

## 🔐 Security Considerations

### Authentication & Authorization
- ✅ JWT tokens for stateless auth
- ✅ Role-based access control (RBAC)
- 🔲 Rate limiting on API endpoints (add middleware)
- 🔲 CSRF protection for state-changing operations
- 🔲 Input validation and sanitization

### File Uploads
- Validate file types (whitelist: jpg, png, webp)
- Limit file sizes (avatar: 5MB, attachments: 10MB)
- Scan uploaded files for malware
- Generate unique filenames to prevent overwrites
- Use signed URLs for private content

### WebSocket Security
- Authenticate WebSocket connections via JWT
- Validate all incoming messages
- Rate limit message sending
- Sanitize message content (prevent XSS)

---

## 📅 Recommended Timeline

| Phase | Features | Duration | Dependencies |
|-------|----------|----------|--------------|
| **Phase 1** | RBAC + Profile Photos | 1 week | None |
| **Phase 2A** | Friend Requests | 3 days | Phase 1 |
| **Phase 2B** | Posts System | 4 days | Phase 2A |
| **Phase 2C** | Comments | 3 days | Phase 2B |
| **Phase 3A** | Groups | 5 days | Phase 1 |
| **Phase 3B** | Organizations | 4 days | Phase 3A |
| **Phase 4A** | Real-time Chat | 1 week | Phase 2A |
| **Phase 4B** | Voice/Murmur | 2 weeks | Phase 3A |

**Total Estimated Time: 6-8 weeks** for full implementation with testing

---

## 🚀 Quick Start Guide

### Step 1: Set Up Replit Integrations
```bash
# Object Storage for file uploads
# Use search_integrations tool to find and install:
# - blueprint:javascript_object_storage

# WebSockets for real-time chat
# - blueprint:javascript_websocket
```

### Step 2: Start with Phase 1, Feature 1.1 (Profile Photos)

**Backend First:**
1. Create `src/repositories/media_repository.cpp`
2. Add object storage configuration
3. Create upload endpoint in `server.cpp`

**Then Frontend:**
1. Create `frontend/app/components/avatar-upload.tsx`
2. Update profile page to show upload button
3. Test upload flow end-to-end

### Step 3: Follow the Phase Plan
Work through features in order, completing backend before frontend for each feature.

---

## 📚 Additional Resources

### Replit Documentation
- Object Storage: https://docs.replit.com/hosting/deployments/object-storage
- WebSockets: https://docs.replit.com/tutorials/websocket-server

### C++ Libraries to Consider
- **nlohmann/json**: JSON parsing (already likely in use)
- **websocketpp**: WebSocket server
- **libcurl**: HTTP client for external APIs
- **opencv**: Image processing for avatar cropping

### Frontend Libraries to Add
```bash
cd frontend
npm install zustand              # State management
npm install react-query          # Data fetching
npm install socket.io-client     # WebSocket client
npm install react-image-crop     # Image cropping
npm install react-dropzone       # File uploads
npm install date-fns             # Date formatting
```

---

## ✅ Success Criteria

**Phase 1 Complete When:**
- ✅ Users can upload and display profile photos
- ✅ Professors can create groups (Students cannot)
- ✅ Permissions are enforced on all protected endpoints

**Phase 2 Complete When:**
- ✅ Users can send, accept, and manage friend requests
- ✅ Users can create posts from their profile
- ✅ Posts appear in friends' feeds based on visibility
- ✅ Users can comment and reply to posts

**Phase 3 Complete When:**
- ✅ Professors can create and manage groups
- ✅ Organizations can be created and managed
- ✅ Group posts are visible only to members

**Phase 4 Complete When:**
- ✅ Friends can send real-time messages
- ✅ Chat shows typing indicators and read receipts
- ✅ Voice calls work for private conversations
- ✅ Public Khave discussions support multiple participants

---

## 🐛 Common Pitfalls to Avoid

1. **Not checking permissions**: Always validate user roles/permissions before actions
2. **Missing indexes**: Add database indexes for foreign keys and frequently queried columns
3. **No pagination**: Always paginate list endpoints to prevent performance issues
4. **Hardcoded URLs**: Use environment variables for API URLs and storage endpoints
5. **No error handling**: Implement comprehensive error handling and user feedback
6. **Memory leaks**: Close database connections and clean up WebSocket listeners
7. **No input validation**: Validate and sanitize all user input on backend
8. **Storing passwords**: Never log or return password hashes in API responses

---

## 💡 Pro Tips

1. **Start Small**: Implement one complete feature end-to-end before moving to the next
2. **Test Early**: Write tests as you go, not after everything is built
3. **Document APIs**: Keep API documentation up-to-date (consider using Swagger/OpenAPI)
4. **User Feedback**: Show loading states, success messages, and helpful error messages
5. **Mobile First**: Design UI for mobile, then enhance for desktop
6. **Incremental Releases**: Deploy working features incrementally rather than all at once
7. **Monitor Performance**: Use browser dev tools and backend logging to identify bottlenecks
8. **Get User Feedback**: Deploy Phase 1-2 first and get real user feedback before Phase 3-4

---

## 📞 Support & Next Steps

This roadmap provides the complete blueprint. The database foundation is ready. Now you can:

1. **Assign to a team**: Break features across frontend/backend developers
2. **Start development**: Follow Phase 1 → Phase 2 → Phase 3 → Phase 4
3. **Iterate**: Launch early phases to users, gather feedback, refine later phases

**Current Status**: ✅ Database Complete | 🔨 Ready for Implementation

Good luck with your implementation! 🚀
