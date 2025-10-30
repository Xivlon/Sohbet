'use client';

import { useEffect, useState, useCallback } from 'react';
import * as React from 'react';
import websocketService, { WebSocketMessage, MessageHandler, ChatMessagePayload } from '../lib/websocket-service';

export type { ChatMessagePayload } from '../lib/websocket-service';

/**
 * Hook to use WebSocket connection with automatic connection management
 */
export function useWebSocket(token: string | null) {
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    if (!token) {
      return;
    }

    // Connect to WebSocket
    websocketService.connect(token).catch(error => {
      console.error('Failed to connect to WebSocket:', error);
    });

    // Listen to connection changes
    const unsubscribe = websocketService.onConnectionChange(setConnected);

    // Cleanup on unmount
    return () => {
      unsubscribe();
      websocketService.disconnect();
    };
  }, [token]);

  return { connected };
}

/**
 * Hook to subscribe to specific WebSocket message types
 */
export function useWebSocketMessage(
  type: string,
  handler: MessageHandler,
  deps: React.DependencyList = []
) {
  useEffect(() => {
    websocketService.on(type, handler);

    return () => {
      websocketService.off(type, handler);
    };
  }, [type, handler, ...deps]);
}

/**
 * Hook to send chat messages
 */
export function useChatWebSocket(
  conversationId: number | null,
  onMessageReceived?: (message: ChatMessagePayload) => void
) {
  const [typingUsers, setTypingUsers] = useState<Set<number>>(new Set());

  // Handle incoming messages
  useWebSocketMessage('chat:message', (message: WebSocketMessage) => {
    const payload = message.payload;
    // Type guard: check if this is a ChatMessagePayload
    if ('conversation_id' in payload && payload.conversation_id === conversationId) {
      console.log('New message received:', payload);
      if (onMessageReceived) {
        onMessageReceived(payload as ChatMessagePayload);
      }
    }
  });

  // Handle typing indicators
  useWebSocketMessage('chat:typing', (message: WebSocketMessage) => {
    const payload = message.payload;
    // Type guard: check if this is a TypingPayload
    if ('conversation_id' in payload && 'user_id' in payload && payload.conversation_id === conversationId) {
      const userId = payload.user_id as number;
      setTypingUsers(prev => {
        const next = new Set(prev);
        next.add(userId);
        
        // Remove typing indicator after 3 seconds
        setTimeout(() => {
          setTypingUsers(current => {
            const updated = new Set(current);
            updated.delete(userId);
            return updated;
          });
        }, 3000);
        
        return next;
      });
    }
  });

  const sendMessage = useCallback((content: string) => {
    if (!conversationId) {
      return false;
    }
    return websocketService.sendChatMessage(conversationId, content);
  }, [conversationId]);

  const sendTyping = useCallback(() => {
    if (!conversationId) {
      return false;
    }
    return websocketService.sendTypingIndicator(conversationId);
  }, [conversationId]);

  return {
    sendMessage,
    sendTyping,
    typingUsers: Array.from(typingUsers)
  };
}

/**
 * Hook to track online users
 */
export function useOnlineUsers() {
  const [onlineUsers, setOnlineUsers] = useState<Set<number>>(new Set());

  // Handle user online status
  useWebSocketMessage('user:online', (message: WebSocketMessage) => {
    if ('user_id' in message.payload) {
      const userId = message.payload.user_id as number;
      setOnlineUsers(prev => {
        const next = new Set(prev);
        next.add(userId);
        return next;
      });
    }
  });

  // Handle user offline status
  useWebSocketMessage('user:offline', (message: WebSocketMessage) => {
    if ('user_id' in message.payload) {
      const userId = message.payload.user_id as number;
      setOnlineUsers(prev => {
        const next = new Set(prev);
        next.delete(userId);
        return next;
      });
    }
  });

  const isOnline = useCallback((userId: number) => {
    return onlineUsers.has(userId);
  }, [onlineUsers]);

  return {
    onlineUsers: Array.from(onlineUsers),
    isOnline
  };
}
