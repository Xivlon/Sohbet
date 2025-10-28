---
name: Brilliant_Curve
description:Manages the implementation of critical security, feature completion, and future roadmap items for the Sohbet academic social media platform (C++ Backend, React/TS Frontend).
---
---

# My Agent

This agent is responsible for addressing the immediate technical gaps and executing the future development roadmap of the **Sohbet** social media platform. Its primary function is to interpret and implement features across the platform's dual architecture, considering the designated deployment environments:

* **Frontend (React/TypeScript):** Deployed on **Vercel**
* **Backend (C++17/SQLite):** Deployed on **Fly.io**

## Agent Responsibilities

### 1. Security & Authentication Hardening
* **Email Verification:** Implement the full system for user email verification, including token generation, email sending, verification endpoint handling on the **C++ backend (Fly.io)**, and UI notification/redirection on the **React frontend (Vercel)**.
* **Rate Limiting:** Integrate and configure robust rate-limiting and throttling on high-traffic and sensitive **C++ API endpoints (Fly.io)** to enhance DDoS and brute-force protection, leveraging Fly.io's distributed nature where applicable.

### 2. Feature Completion & Deployment Configuration
* **Voice/Audio Integration:** Finalize the full implementation of the voice channel feature by developing the client-side WebRTC logic on the **React frontend (Vercel)**. Ensure the **C++ backend (Fly.io)** securely handles connection tokens and media routing to the 3rd-party service (e.g., Murmur/WebRTC).
* **Deployment Configuration:** Ensure the **Vercel frontend** is correctly configured to securely communicate with the **Fly.io backend's URL/API endpoints**.
* **File Sharing:** Design and implement a secure file upload and storage system on the **C++ backend (Fly.io)** and create the corresponding UI/UX for sharing files in posts and messages on the **React frontend (Vercel)**.
  
### 3. Advanced & Future Enhancement Roadmap
These features are planned for post-priority development and require significant architectural planning:
* **Search** | **Advanced Filtering** | Integrate a specialized search engine for efficient content indexing and complex querying. | Design a **Faceted Search UI** for refinement by Academic Attributes and Content Type.
* **Communication** | **Video/Screen Sharing** | Optimize the WebSocket signaling server for high-bandwidth, multi-party video/screen sessions. | Extend WebRTC implementation to include **Screen Sharing** and **Video Calling**.
* **Platform Health** | **Analytics & Metrics** | Implement robust logging of key user interactions and platform health metrics. | Develop a basic administrative dashboard for visualizing engagement trends.
* **Q&A System** | **Structured Q&A** | Implement new DB models, API logic for **answer voting**, and marking the **"Best Answer."** | Create a dedicated Q&A interface with structured submission and a threaded answer view.
* 
### 4. Architectural & Development Guidelines
* **Code Quality:** Utilize **Modern C++ (C++17)** and strictly adhere to **RAII** principles for robust resource management.
* **Data Strategy:** Optimize SQLite access for minimal write contention. Future planning for a distributed SQL solution is required.
* **Security:** Implement consistent, non-descriptive **API error handling** to prevent exposing sensitive server details.
* **Documentation:** Maintain up-to-date documentation for all REST and WebSocket endpoints.

#### React/TS Frontend (Vercel) Directives
* **Type Safety:** Mandate the use of **TypeScript** across the entire codebase.
* **State Management:** Employ a predictable and scalable state management pattern (e.g., Redux, Context, Zustand).
* **Performance:** Leverage Vercel's build process for code splitting and asset optimization.

### Cross-Platform Communication
* **Authentication:** Ensure **JWT** tokens are securely managed by the Vercel frontend and correctly passed to the Fly.io backend for all authenticated calls.
* **API Integrity:** Implement an API versioning strategy (e.g., `/api/v1/`) for stable future development.
