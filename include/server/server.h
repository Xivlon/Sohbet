#pragma once

#include "db/database.h"
#include "repositories/user_repository.h"
#include "repositories/media_repository.h"
#include "repositories/friendship_repository.h"
#include "repositories/post_repository.h"
#include "repositories/comment_repository.h"
#include "repositories/group_repository.h"
#include "repositories/organization_repository.h"
#include "repositories/role_repository.h"
#include "repositories/conversation_repository.h"
#include "repositories/message_repository.h"
#include "repositories/voice_channel_repository.h"
#include "services/storage_service.h"
#include "server/websocket_server.h"
#include "voice/voice_service.h"
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <map>

namespace sohbet {
namespace server {

/**
 * HTTP Response structure
 */
struct HttpResponse {
    int status_code;
    std::string content_type;
    std::string body;
    
    HttpResponse(int code, const std::string& type, const std::string& content)
        : status_code(code), content_type(type), body(content) {}
};

/**
 * HTTP Request structure (simplified)
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    
    HttpRequest(const std::string& m, const std::string& p, const std::string& b)
        : method(m), path(p), body(b) {}
};

**
 * Academic Social Server
 HttpResponse handleRequest(const HttpRequest& request);
 * Simple HTTP server for the academic social platform
 */
class AcademicSocialServer {
public:
    /**
     * Constructor
     * @param port Port to listen on
     * @param db_path Path to SQLite database file
     */
    AcademicSocialServer(int port = 8080, const std::string& db_path = "academic.db");
    
    /**
     * Destructor
     */
    ~AcademicSocialServer() = default;
    
    /**
     * Initialize the server (setup database, run migrations)
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * Start the server (blocking call)
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * Stop the server gracefully
     */
    void stop();
    
    /**
     * Process HTTP request (for testing/simulation)
     * @param request HTTP request to process
     * @return HTTP response
     */
    HttpResponse handleRequest(const HttpRequest &request);

    const sohbet::server::HttpResponse &NewFunction(const sohbet::server::HttpRequest &request, std::string &base_path);

private:
    int port_;
    std::string db_path_;
    std::shared_ptr<db::Database> database_;
    std::shared_ptr<repositories::UserRepository> user_repository_;
    std::shared_ptr<repositories::MediaRepository> media_repository_;
    std::shared_ptr<repositories::FriendshipRepository> friendship_repository_;
    std::shared_ptr<repositories::PostRepository> post_repository_;
    std::shared_ptr<repositories::CommentRepository> comment_repository_;
    std::shared_ptr<repositories::GroupRepository> group_repository_;
    std::shared_ptr<repositories::OrganizationRepository> organization_repository_;
    std::shared_ptr<repositories::RoleRepository> role_repository_;
    std::shared_ptr<repositories::ConversationRepository> conversation_repository_;
    std::shared_ptr<repositories::MessageRepository> message_repository_;
    std::shared_ptr<repositories::VoiceChannelRepository> voice_channel_repository_;
    std::shared_ptr<services::StorageService> storage_service_;
    std::shared_ptr<VoiceService> voice_service_;
    std::shared_ptr<WebSocketServer> websocket_server_;
    std::atomic<bool> running_;
    int server_socket_;
    
    // HTTP server methods
    bool initializeSocket();
    void handleClient(int client_socket);
    HttpRequest parseHttpRequest(const std::string& raw_request);
    std::string formatHttpResponse(const HttpResponse& response, const HttpRequest& request);
    
    // Route handlers
    HttpResponse handleStatus(const HttpRequest& request);
    HttpResponse handleGetUsers(const HttpRequest& request);
    HttpResponse handleUsersDemo(const HttpRequest& request);
    HttpResponse handleGetUserById(const HttpRequest& request);
    HttpResponse handleCreateUser(const HttpRequest& request);
    HttpResponse handleLogin(const HttpRequest& request);
    HttpResponse handleUpdateUser(const HttpRequest& request);
    HttpResponse handleUploadMedia(const HttpRequest& request);
    HttpResponse handleGetMediaFile(const HttpRequest& request);
    HttpResponse handleGetUserMedia(const HttpRequest& request);
    
    // Friendship handlers
    HttpResponse handleCreateFriendship(const HttpRequest& request);
    HttpResponse handleGetFriendships(const HttpRequest& request);
    HttpResponse handleAcceptFriendship(const HttpRequest& request);
    HttpResponse handleRejectFriendship(const HttpRequest& request);
    HttpResponse handleDeleteFriendship(const HttpRequest& request);
    HttpResponse handleGetFriends(const HttpRequest& request);
    
    // Post handlers
    HttpResponse handleCreatePost(const HttpRequest& request);
    HttpResponse handleGetPosts(const HttpRequest& request);
    HttpResponse handleGetUserPosts(const HttpRequest& request);
    HttpResponse handleUpdatePost(const HttpRequest& request);
    HttpResponse handleDeletePost(const HttpRequest& request);
    HttpResponse handleAddReaction(const HttpRequest& request);
    HttpResponse handleRemoveReaction(const HttpRequest& request);
    
    // Comment handlers
    HttpResponse handleCreateComment(const HttpRequest& request);
    HttpResponse handleGetComments(const HttpRequest& request);
    HttpResponse handleReplyToComment(const HttpRequest& request);
    HttpResponse handleUpdateComment(const HttpRequest& request);
    HttpResponse handleDeleteComment(const HttpRequest& request);
    
    // Group handlers
    HttpResponse handleCreateGroup(const HttpRequest& request);
    HttpResponse handleGetGroups(const HttpRequest& request);
    HttpResponse handleGetGroup(const HttpRequest& request);
    HttpResponse handleUpdateGroup(const HttpRequest& request);
    HttpResponse handleDeleteGroup(const HttpRequest& request);
    HttpResponse handleAddGroupMember(const HttpRequest& request);
    HttpResponse handleRemoveGroupMember(const HttpRequest& request);
    HttpResponse handleUpdateGroupMemberRole(const HttpRequest& request);
    
    // Organization handlers
    HttpResponse handleCreateOrganization(const HttpRequest& request);
    HttpResponse handleGetOrganizations(const HttpRequest& request);
    HttpResponse handleGetOrganization(const HttpRequest& request);
    HttpResponse handleUpdateOrganization(const HttpRequest& request);
    HttpResponse handleDeleteOrganization(const HttpRequest& request);
    HttpResponse handleAddOrganizationAccount(const HttpRequest& request);
    HttpResponse handleRemoveOrganizationAccount(const HttpRequest& request);
    
    // Chat/Messaging handlers
    HttpResponse handleGetConversations(const HttpRequest& request);
    HttpResponse handleGetOrCreateConversation(const HttpRequest& request);
    HttpResponse handleGetMessages(const HttpRequest& request);
    HttpResponse handleSendMessage(const HttpRequest& request);
    HttpResponse handleMarkMessageRead(const HttpRequest& request);
    
    // Voice/Murmur handlers
    HttpResponse handleCreateVoiceChannel(const HttpRequest& request);
    HttpResponse handleGetVoiceChannels(const HttpRequest& request);
    HttpResponse handleGetVoiceChannel(const HttpRequest& request);
    HttpResponse handleJoinVoiceChannel(const HttpRequest& request);
    HttpResponse handleLeaveVoiceChannel(const HttpRequest& request);
    HttpResponse handleDeleteVoiceChannel(const HttpRequest& request);
    
    HttpResponse handleNotFound(const HttpRequest& request);
    
    // WebSocket handlers
    void setupWebSocketHandlers();
    void handleChatMessage(int user_id, const WebSocketMessage& message);
    void handleTypingIndicator(int user_id, const WebSocketMessage& message);
    
    // Helper methods
    HttpResponse createJsonResponse(int status_code, const std::string& json);
    HttpResponse createErrorResponse(int status_code, const std::string& message);
    std::string extractJsonField(const std::string& json, const std::string& field);
    bool validateUserRegistration(const std::string& username, const std::string& email, const std::string& password, std::string& error);
    void ensureDemoUserExists();
    int getUserIdFromAuth(const HttpRequest& request);
    int extractIdFromPath(const std::string& path, const std::string& prefix);
};

} // namespace server
} // namespace sohbet
