# Academic Features Development Plan

## Overview
This document outlines the development roadmap for Sohbet's academic social media features, focusing on university-specific functionality and educational collaboration tools.

## Development Phases

### Phase 1: Foundation ✅ COMPLETED
**Scope**: Basic project structure and core infrastructure
- [x] C++ project setup with CMake
- [x] Basic class structure planning
- [x] Initial repository structure

### Phase 2: User Management & Database ✅ COMPLETED
**Scope**: User registration, authentication foundation, and database persistence

#### Increment 1: Core Models ✅
- [x] User model with academic profile fields
- [x] JSON serialization (excluding sensitive data)
- [x] Input validation for usernames, emails, passwords

#### Increment 2: Database & Registration ✅
- [x] SQLite RAII database wrapper
- [x] UserRepository with CRUD operations
- [x] Database migrations system
- [x] User registration endpoint (POST /api/users)
- [x] Placeholder password hashing (marked for security upgrade)
- [x] Comprehensive test suite

**Key Features Delivered:**
- User registration with academic fields (university, department, enrollment year, languages)
- SQLite database with proper schema
- Password hashing (placeholder implementation)
- RESTful API endpoints
- Input validation and uniqueness constraints
- Error handling with appropriate HTTP status codes

**Security Implementation:**
- Placeholder hashing using std::hash + salt (INSECURE - marked for replacement)
- Password hashes never exposed in API responses
- SQL injection prevention through prepared statements
- Input validation for all user fields

### Phase 3: Authentication & Authorization 🔄 IN PROGRESS
**Scope**: Secure login system and token-based authentication

#### Increment 3: Secure Authentication (Next)
- [ ] Replace placeholder hashing with bcrypt or Argon2
- [ ] Login endpoint with credential verification
- [ ] JWT token generation and validation
- [ ] Password reset functionality
- [ ] Session management

#### Increment 4: User Management
- [ ] GET /api/users/:username endpoint
- [ ] User profile updates
- [ ] Account deactivation
- [ ] Admin user roles

### Phase 4: Academic Core Models 📋 PLANNED
**Scope**: Course and study group management

#### Increment 5: Course System
- [ ] Course model (code, name, instructor, semester)
- [ ] Course enrollment system
- [ ] Course repository and API endpoints
- [ ] Student-course relationship management

#### Increment 6: Study Groups
- [ ] StudyGroup model with course associations
- [ ] Group creation and membership management
- [ ] Group privacy settings (public, private, invite-only)
- [ ] Group administration features

### Phase 5: Content & Collaboration 📝 PLANNED
**Scope**: Posts, discussions, and academic content sharing

#### Increment 7: Discussion System
- [ ] Post model for course discussions
- [ ] Comment threading
- [ ] Voting/rating system
- [ ] Content moderation tools

#### Increment 8: Q&A System
- [ ] Question and Answer models
- [ ] Academic Q&A with tagging
- [ ] Best answer selection
- [ ] Reputation system

### Phase 6: Advanced Features 🚀 FUTURE
**Scope**: Real-time features and advanced collaboration

#### Increment 9: Real-time Communication
- [ ] WebSocket integration
- [ ] Real-time messaging
- [ ] Live study sessions
- [ ] Notification system

#### Increment 10: File & Resource Sharing
- [ ] File upload and sharing
- [ ] Academic resource library
- [ ] Document collaboration
- [ ] Version control for shared documents

## Technical Implementation Status

### Database Schema Evolution
**Current Tables:**
- `users` - User profiles with academic information

**Planned Tables:**
- `courses` - Course information and metadata
- `study_groups` - Study group definitions
- `enrollments` - Student-course relationships
- `group_memberships` - User-group relationships
- `posts` - Discussion posts and content
- `comments` - Threaded discussion comments
- `questions` - Q&A questions
- `answers` - Q&A answers

### Security Roadmap

#### Current Security Status
- ❌ **CRITICAL**: Placeholder password hashing (std::hash + salt)
- ✅ Input validation and sanitization
- ✅ SQL injection prevention
- ✅ Password hash exclusion from API responses
- ✅ Basic error handling

#### Security Improvements Needed
- 🔴 **HIGH PRIORITY**: Implement bcrypt or Argon2 password hashing
- 🔴 **HIGH PRIORITY**: Add JWT-based authentication
- 🟡 **MEDIUM**: Rate limiting for API endpoints
- 🟡 **MEDIUM**: Audit logging system
- 🟡 **MEDIUM**: Input sanitization enhancements
- 🟢 **LOW**: Advanced session management

### Testing Strategy
- ✅ Unit tests for models and repositories
- ✅ Integration tests for database operations
- ✅ API endpoint testing framework
- [ ] Load testing for concurrent users
- [ ] Security penetration testing
- [ ] End-to-end testing scenarios

### Documentation Status
- ✅ API documentation with examples
- ✅ Build and deployment instructions
- ✅ Security warnings and disclaimers
- [ ] Architecture documentation
- [ ] Database migration guides
- [ ] Contribution guidelines

## Next Sprint Planning

### Immediate Priorities (Phase 3, Increment 3)
1. **Security Hardening** - Replace placeholder hashing
2. **Authentication** - Implement login endpoint
3. **JWT Integration** - Token-based authentication
4. **Testing** - Expand test coverage for auth flows

### Success Criteria for Next Release (v0.3.0)
- Secure password hashing implementation
- Working login/logout functionality
- JWT token generation and validation
- Comprehensive authentication tests
- Updated security documentation

## Risk Assessment

### High Risk Items
- **Password Security**: Current implementation is completely insecure
- **Authentication Gap**: No login mechanism yet implemented
- **Scale Preparation**: Database design needs optimization planning

### Mitigation Strategies
- Prioritize security improvements in next sprint
- Implement comprehensive testing before any user-facing deployment
- Plan for database migration strategy early
- Regular security audits of implemented features

## Learning Objectives (Academic Context)
This project serves as a practical learning platform for:
- C++ backend development patterns
- Database design and ORM concepts
- RESTful API design principles
- Security best practices in web applications
- Test-driven development methodologies
- Project planning and documentation practices