# Features Needing Attention

**Date**: October 30, 2025  
**Version**: 0.3.0-academic  
**Based On**: Comprehensive checkup report (Issues.md)

---

## Executive Summary

While Sohbet has achieved ~95% implementation of the roadmap with excellent backend stability, **critical issues prevent production deployment**. This document prioritizes issues by severity and provides actionable remediation steps.

**Overall Assessment**: ⚠️ **Development Ready** - Core functionality works, but requires fixes for production readiness.

---

## Critical Issues (P0) - MUST FIX BEFORE PRODUCTION

### 1. 🔴 Hardcoded JWT Secret - SECURITY VULNERABILITY

**Severity**: Critical  
**Impact**: Anyone can generate valid JWT tokens and impersonate users  
**Location**: `include/security/jwt.h:19, 22`

**Current Code**:
```cpp
std::string generate_jwt_token(..., const std::string& secret = "default_secret", ...)
std::optional<JWTPayload> verify_jwt_token(..., const std::string& secret = "default_secret")
```

**Remediation** (Estimated: 1 hour):
1. Remove default parameter from JWT functions
2. Read secret from environment variable:
   ```cpp
   const char* jwt_secret = std::getenv("SOHBET_JWT_SECRET");
   if (!jwt_secret || strlen(jwt_secret) < 32) {
       throw std::runtime_error("SOHBET_JWT_SECRET must be set and at least 32 characters");
   }
   ```
3. Create `.env.example` with:
   ```
   SOHBET_JWT_SECRET=your-secret-key-here-min-32-chars
   ```
4. Update documentation to require JWT secret in deployment

**Code Note**: The implementation includes comment: "Simple JWT token utilities (for educational/demo purposes). In production, use a proper JWT library"

**Long-term Recommendation**: Replace with production-ready JWT library (e.g., jwt-cpp)

---

### 2. 🔴 Frontend Build Failures - APPLICATION NON-FUNCTIONAL

**Severity**: Critical  
**Impact**: Application cannot be built or deployed  
**Status**: ❌ BUILD FAILED

**Missing Files** (Estimated: 2 hours):
- `frontend/app/components/ui/dialog.tsx`
- `frontend/app/components/ui/label.tsx`
- `frontend/app/components/ui/skeleton.tsx`

**Action Required**:
1. Create missing Shadcn UI components using CLI:
   ```bash
   cd frontend
   npx shadcn-ui@latest add dialog
   npx shadcn-ui@latest add label
   npx shadcn-ui@latest add skeleton
   ```

**Incorrect Import Paths** (Estimated: 30 minutes):
- Location: `frontend/app/messages/page.tsx`
- Issues:
  ```typescript
  // ❌ WRONG
  import { Card } from '@/components/card'
  import { ChatList } from '@/components/chat-list'
  import { ChatWindow } from '@/components/chat-window'
  
  // ✅ CORRECT
  import { Card } from '@/app/components/ui/card'
  import { ChatList } from '@/app/components/chat-list'
  import { ChatWindow } from '@/app/components/chat-window'
  ```

**Action Required**:
1. Fix import paths in `messages/page.tsx`
2. Standardize on one import alias pattern across codebase

---

### 3. 🔴 Missing UI Components

**Severity**: Critical  
**Impact**: Build failures, missing functionality  
**Files**: dialog.tsx, label.tsx, skeleton.tsx

See issue #2 above for remediation steps.

---

## High Priority Issues (P1) - FIX BEFORE BETA RELEASE

### 4. 🟠 TypeScript `any` Types - TYPE SAFETY ISSUES

**Severity**: High  
**Impact**: Loss of type safety, potential runtime errors  
**Count**: 11 instances

**Locations**:
- `chat-window.tsx:40` (message type)
- `friends-list.tsx:58` (friend type)
- `use-websocket.ts:41, 57` (WebSocket handlers)
- `websocket-service.ts:15, 110` (service methods)
- `messages/page.tsx:26` (conversation type)

**Remediation** (Estimated: 2 hours):
1. Define proper TypeScript interfaces:
   ```typescript
   interface Message {
     id: number;
     conversation_id: number;
     sender_id: number;
     content: string;
     created_at: string;
   }
   
   interface Friend {
     id: number;
     username: string;
     email: string;
     avatar_url?: string;
   }
   ```
2. Replace all `any` types with proper interfaces
3. Enable strict TypeScript mode

---

### 5. 🟠 Import Path Inconsistencies - BUILD RELIABILITY

**Severity**: High  
**Impact**: Confusing codebase, potential build issues  
**Pattern**: Mixed use of `@/components` and `@/app/components`

**Remediation** (Estimated: 1 hour):
1. Standardize on single import pattern (recommend: `@/app/components`)
2. Update `tsconfig.json` paths configuration
3. Run find/replace across codebase
4. Add ESLint rule to enforce consistent imports

---

### 6. 🟠 React setState in useEffect - PERFORMANCE ISSUES

**Severity**: High  
**Impact**: Cascading renders, poor performance  
**Location**: `websocket-provider.tsx:22, 46`

**Issue**: setState called within useEffect can cause infinite render loops

**Remediation** (Estimated: 1 hour):
1. Review WebSocket provider implementation
2. Move state updates outside useEffect where possible
3. Use proper dependency arrays
4. Consider using refs for non-reactive values

---

### 7. 🟠 Rate Limiting - API ABUSE PREVENTION

**Severity**: High  
**Impact**: Vulnerable to DoS attacks, resource exhaustion  
**Status**: Not implemented

**Remediation** (Estimated: 3 hours):
1. Implement rate limiting middleware in C++ backend
2. Use token bucket or sliding window algorithm
3. Configure limits per endpoint:
   - Login: 5 requests/minute
   - Registration: 3 requests/hour
   - API calls: 100 requests/minute
4. Add rate limit headers in responses:
   ```
   X-RateLimit-Limit: 100
   X-RateLimit-Remaining: 95
   X-RateLimit-Reset: 1635724800
   ```

---

## Medium Priority Issues (P2) - ADDRESS WITHIN 2 WEEKS

### 8. 🟡 Compiler Warnings - CODE QUALITY

**Severity**: Medium  
**Impact**: Code maintainability  
**Count**: 6 instances

**Unused Parameters**:
- `src/models/role.cpp:32` - unused parameter 'json' in fromJson()
- `src/models/friendship.cpp:35` - unused parameter 'json' in fromJson()
- `src/models/post.cpp:55` - unused parameter 'json' in fromJson()
- `src/models/comment.cpp:53` - unused parameter 'json' in fromJson()
- `src/repositories/conversation_repository.cpp:70` - unused variable 'conversation_id'
- `tests/test_websocket_server.cpp:17` - unused parameters in lambda functions

**Remediation** (Estimated: 30 minutes):
1. Mark unused parameters with `[[maybe_unused]]` or `(void)param;`
2. Remove truly unused code
3. Enable `-Werror` to treat warnings as errors

---

### 9. 🟡 Unused Imports - CODE CLEANLINESS

**Severity**: Medium  
**Impact**: Code clarity, bundle size  
**Count**: 15 instances

**Remediation** (Estimated: 30 minutes):
1. Run ESLint with `--fix` flag to auto-remove
2. Configure editor to highlight unused imports
3. Add pre-commit hook to prevent new unused imports

---

### 10. 🟡 useEffect Dependencies - POTENTIAL BUGS

**Severity**: Medium  
**Impact**: Stale closures, unexpected behavior  
**Count**: 12 missing dependency warnings

**Remediation** (Estimated: 1 hour):
1. Review each useEffect hook
2. Add missing dependencies to dependency array
3. Use `useCallback` and `useMemo` where appropriate
4. Enable ESLint rule `react-hooks/exhaustive-deps` as error

---

## Low Priority Issues (P3) - NICE TO HAVE

### 11. 🟢 Image Crop Functionality for Avatars

**Status**: Pending  
**Impact**: Better user experience

**Remediation** (Estimated: 4 hours):
1. Install react-image-crop library
2. Create crop modal component
3. Integrate into avatar upload flow
4. Add preview functionality

---

### 12. 🟢 Email Verification System

**Status**: Not implemented  
**Impact**: Account security, spam prevention

**Remediation** (Estimated: 8 hours):
1. Add email verification token to database
2. Implement email sending service (SMTP)
3. Create verification endpoint
4. Add UI for resending verification email
5. Block certain features until verified

---

### 13. 🟢 CSRF Protection

**Status**: Not implemented  
**Impact**: Security hardening

**Remediation** (Estimated: 2 hours):
1. Implement CSRF token generation
2. Add token to forms and state-changing requests
3. Validate tokens on backend
4. Add to security headers

---

### 14. 🟢 Frontend Tests

**Status**: 0% coverage  
**Impact**: Code reliability, regression prevention

**Remediation** (Estimated: 8 hours):
1. Set up Jest + React Testing Library
2. Add test configuration
3. Write unit tests for components
4. Write integration tests for user flows
5. Set up CI to run tests on PR

---

## Long-Term Improvements

### 15. 🟢 Voice/WebRTC Integration (95% Complete)

**Status**: Foundation complete, production-ready
**Estimated Effort**: Optional enhancements 2-3 weeks

**What's Done**:
- ✅ Voice channel database schema
- ✅ VoiceChannelRepository
- ✅ Voice channel REST API (6 endpoints)
- ✅ WebRTC peer-to-peer communication
- ✅ Khave UI component (fully functional)
- ✅ WebRTC signaling via WebSocket
- ✅ ICE server configuration (STUN/TURN support)
- ✅ JWT-based connection token authentication
- ✅ Real-time audio/video streaming

**Optional Enhancements**:
- Screen sharing functionality
- Call recording (with user consent)
- Mobile native app support (iOS/Android)
- Group conference calling
- Real-time transcription integration
- Third-party service integration (Daily.co, Twilio, Agora)

---

### 16. 🔵 Production Deployment Setup

**Status**: Not configured  
**Estimated Effort**: 1 week

**What's Needed**:
1. Docker configuration for backend
2. Docker Compose for full stack
3. GitHub Actions CI/CD pipeline
4. Environment variable management
5. Health check endpoints
6. Error tracking (e.g., Sentry)
7. Reverse proxy configuration (nginx)
8. SSL/TLS certificate setup
9. Database migration strategy
10. Monitoring and logging

---

## Summary Statistics

### Issues by Severity:
- **P0 (Critical)**: 3 issues (~3.5 hours to fix)
- **P1 (High)**: 4 issues (~7 hours to fix)
- **P2 (Medium)**: 3 issues (~2 hours to fix)
- **P3 (Low)**: 4 issues (~22 hours to implement)
- **Complete**: Voice/WebRTC integration (production-ready)
- **Optional**: 1 major project for advanced features (~2-3 weeks)

### Total Effort to Production Ready:
- **Critical fixes**: 3.5 hours
- **High priority**: 7 hours
- **Medium priority**: 2 hours
- **Total P0-P2**: ~12.5 hours (1-2 days)
- **Voice/WebRTC**: Already complete, production-ready

### Backend Stability: ⭐⭐⭐⭐½ (4.5/5)
- All tests pass
- Runtime stable
- Minor warnings only
- **Blocker**: JWT secret issue

### Frontend Stability: ⭐⭐½ (2.5/5)
- Modern stack
- Good architecture
- **Blocker**: Build failures
- **Blocker**: Import issues

### Overall Stability: ⭐⭐⭐½ (3.5/5)
- **Production Ready**: No (requires critical fixes)
- **Development Ready**: Yes (backend fully functional)
- **Demo Ready**: Partial (backend can be demoed)

---

## Recommended Action Plan

### Week 1: Critical Fixes (Days 1-2)
1. ✅ Fix JWT secret security issue (1 hour)
2. ✅ Create missing UI components (2 hours)
3. ✅ Fix import paths (30 minutes)
4. ✅ Test frontend build (30 minutes)

### Week 1: High Priority (Days 3-5)
5. ✅ Fix TypeScript `any` types (2 hours)
6. ✅ Standardize import paths (1 hour)
7. ✅ Fix React setState issues (1 hour)
8. ✅ Implement rate limiting (3 hours)

### Week 2: Medium Priority
9. ✅ Resolve compiler warnings (30 minutes)
10. ✅ Clean up unused imports (30 minutes)
11. ✅ Fix useEffect dependencies (1 hour)
12. ✅ Add CORS configuration (1 hour)

### Week 3-4: Low Priority & Testing
13. ✅ Add frontend test suite (8 hours)
14. ✅ Implement CSRF protection (2 hours)
15. ✅ Add API documentation (4 hours)
16. ✅ Security audit (4 hours)

### Month 2+: Long-term Improvements
17. Voice/Murmur integration (2-3 weeks)
18. Production deployment setup (1 week)
19. Email verification (1 week)

---

## Resources & References

### Documentation:
- [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) - Complete roadmap
- [ROADMAP_STATUS_CHECK.md](ROADMAP_STATUS_CHECK.md) - Detailed status
- [Issues.md](Issues.md) - Comprehensive checkup report
- [PHASE4A_COMPLETION_REPORT.md](PHASE4A_COMPLETION_REPORT.md) - WebSocket chat completion
- [PHASE4C_COMPLETION_REPORT.md](PHASE4C_COMPLETION_REPORT.md) - Voice API completion

### External Resources:
- JWT Best Practices: https://tools.ietf.org/html/rfc8725
- React Testing Library: https://testing-library.com/react
- Shadcn UI Components: https://ui.shadcn.com/
- Rate Limiting Algorithms: https://en.wikipedia.org/wiki/Rate_limiting

---

## Monitoring Progress

Track completion of these issues:

### Critical Issues (P0):
- [ ] JWT secret from environment variable
- [ ] Create missing UI components
- [ ] Fix import path inconsistencies

### High Priority (P1):
- [ ] Fix TypeScript `any` types
- [ ] Standardize import patterns
- [ ] Fix React setState in useEffect
- [ ] Implement rate limiting

### Medium Priority (P2):
- [ ] Resolve compiler warnings
- [ ] Clean up unused imports
- [ ] Fix useEffect dependencies

### Low Priority (P3):
- [ ] Image crop functionality
- [ ] Email verification
- [ ] CSRF protection
- [ ] Frontend test suite

---

**Last Updated**: October 30, 2025  
**Next Review**: After completing P0 and P1 issues  
**Owner**: Development Team
