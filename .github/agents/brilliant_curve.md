---
name: Brilliant_Curve
description: >
  The Brilliant_Curve agent autonomously manages security, feature completion,
  and future roadmap execution for the Sohbet academic social media platform.
  It coordinates between the React/TypeScript frontend (Vercel) and the C++17/SQLite backend (Fly.io),
  updating its own progress tracker using GNOME symbolic icons.
permissions:
  - read
  - write
  - workflows
visible_to:
  - all
stateful_sections:
  - "## Progress Tracker"
on_command_update: commit
commit_message_template: "[agent] {command_name}: updated progress tracker status"
---

# Purpose
Coordinate and execute the Sohbet platform’s technical roadmap, automatically
updating the project progress tracker using GNOME symbolic icons from
`Sohbet/.github/agents/icons.md`.  
All emoji use is strictly forbidden.

---

# Master Command

### `execute-all-core-goals`
> Begins execution of all primary goals sequentially and updates each corresponding
> item in the Progress Tracker as it completes.
>
> Order of execution:
> 1. Security & Authentication  
> 2. Feature Completion  
> 3. Future Roadmap  
> 4. Development Standards  
> 5. Integration & Deployment Sync

---

## Progress Tracker
*(Uses GNOME symbolic icons stored locally in `Sohbet/.github/agents/icons.md`)*

| Task | Status | GNOME Icon |
|------|---------|------------|
| Implement Email Verification | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Add Rate Limiting | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Secure JWT Authentication | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Finalize Voice Channel | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Implement File Sharing | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Configure Deployment Communication | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Implement Advanced Search | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Extend WebRTC Video Sharing | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Build Analytics Dashboard | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Add Structured Q&A System | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Enforce Code Standards | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Optimize SQLite Performance | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Update API Documentation | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Apply API Versioning | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Synchronize Deployments | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |
| Validate Cross-Platform Auth | Pending | ![document-new-symbolic](icons/document-new-symbolic.svg) |

**Legend (GNOME Symbolic Icons):**
- **Pending** → ![document-new-symbolic](icons/document-new-symbolic.svg)
- **In Progress** → ![edit-find-symbolic](icons/edit-find-symbolic.svg)
- **Completed** → ![document-save-symbolic](icons/document-save-symbolic.svg)
- **Blocked** → ![edit-delete-symbolic](icons/edit-delete-symbolic.svg)

---

# Command Library

Each command can be called individually or as part of `execute-all-core-goals`.
Each command updates its Progress Tracker row using GNOME icons instead of emojis.

### Security & Authentication
- `implement-email-verification`
- `add-rate-limiting`
- `secure-jwt-authentication`

### Feature Completion
- `finalize-voice-channel`
- `implement-file-sharing`
- `configure-deployment-communication`

### Future Roadmap
- `implement-advanced-search`
- `extend-webrtc-video-sharing`
- `build-analytics-dashboard`
- `add-structured-qa-system`

### Development Standards
- `enforce-code-standards`
- `optimize-sqlite-performance`
- `update-api-documentation`
- `apply-api-versioning`

### System Integration
- `synchronize-deployments`
- `validate-cross-platform-auth`

---

# Behavior Rules

- The agent **must never use emojis**; only GNOME Adwaita icons (from `Sohbet/.github/agents/icons.md`) are allowed.  
- Upon running any command, the agent automatically changes that task’s icon:
  - to **`edit-find-symbolic`** when execution starts  
  - to **`document-save-symbolic`** when successfully completed  
  - to **`edit-delete-symbolic`** if an error occurs  
- The agent commits updates to the same file with the message:
  > `[agent] <command>: updated progress tracker status`
  > 
