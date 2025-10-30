/**
 * WebSocket Service for real-time communication
 * Manages WebSocket connections, message handling, and reconnection logic
 */
'use client';

export type WebSocketMessageType = 
  | 'chat:send'
  | 'chat:message'
  | 'chat:typing'
  | 'user:online'
  | 'user:offline';

export interface ChatMessagePayload {
  id?: number;
  conversation_id: number;
  sender_id: number;
  content: string;
  created_at?: string;
}

export interface TypingPayload {
  conversation_id: number;
  user_id: number;
  username: string;
}

export interface UserStatusPayload {
  user_id: number;
  username: string;
}

export type MessagePayload = ChatMessagePayload | TypingPayload | UserStatusPayload | Record<string, unknown>;

export interface WebSocketMessage {
  type: WebSocketMessageType;
  payload: MessagePayload;
}

export type MessageHandler = (message: WebSocketMessage) => void;

class WebSocketService {
  private ws: WebSocket | null = null;
  private url: string;
  private token: string | null = null;
  private handlers: Map<string, Set<MessageHandler>> = new Map();
  private reconnectAttempts = 0;
  private maxReconnectAttempts = 5;
  private reconnectDelay = 1000;
  private isConnecting = false;
  private connectionListeners: Set<(connected: boolean) => void> = new Set();

  constructor(url?: string) {
    // Use environment variable for WebSocket URL
    const envWsUrl = process.env.NEXT_PUBLIC_WS_URL;
    
    if (url) {
      this.url = url;
    } else if (envWsUrl) {
      // If NEXT_PUBLIC_WS_URL is set, use it
      // Convert https:// to wss:// if needed
      this.url = envWsUrl.replace(/^https:\/\//, 'wss://').replace(/^http:\/\//, 'ws://');
    } else {
      // Fallback: Only access window on client-side
      const hostname = typeof window !== 'undefined' ? window.location.hostname : '0.0.0.0';
      const protocol = typeof window !== 'undefined' && window.location.protocol === 'https:' ? 'wss:' : 'ws:';
      this.url = `${protocol}//${hostname}:8081`;
    }
  }

  /**
   * Connect to WebSocket server with authentication token
   */
  connect(token: string): Promise<void> {
    if (this.ws?.readyState === WebSocket.OPEN) {
      console.log('WebSocket already connected');
      return Promise.resolve();
    }

    if (this.isConnecting) {
      console.log('WebSocket connection already in progress');
      return Promise.resolve();
    }

    this.token = token;
    this.isConnecting = true;

    return new Promise((resolve, reject) => {
      try {
        // Include token in URL as query parameter
        const wsUrl = `${this.url}/?token=${encodeURIComponent(token)}`;
        
        // Check for insecure connection on HTTPS pages
        if (typeof window !== 'undefined' && 
            window.location.protocol === 'https:' && 
            wsUrl.startsWith('ws://')) {
          const error = new Error(
            'Cannot establish insecure WebSocket connection (ws://) from secure page (https://). ' +
            'Please configure NEXT_PUBLIC_WS_URL environment variable with a secure WebSocket URL (wss://)'
          );
          console.error(error.message);
          reject(error);
          return;
        }
        
        this.ws = new WebSocket(wsUrl);

        this.ws.onopen = () => {
          console.log('✓ WebSocket connected');
          this.isConnecting = false;
          this.reconnectAttempts = 0;
          this.notifyConnectionListeners(true);
          resolve();
        };

        this.ws.onmessage = (event) => {
          try {
            const message: WebSocketMessage = JSON.parse(event.data);
            this.handleMessage(message);
          } catch (error) {
            console.error('Failed to parse WebSocket message:', error);
          }
        };

        this.ws.onerror = (error) => {
          console.error('WebSocket error:', error);
          this.isConnecting = false;
          reject(error);
        };

        this.ws.onclose = () => {
          console.log('WebSocket disconnected');
          this.isConnecting = false;
          this.notifyConnectionListeners(false);
          this.attemptReconnect();
        };
      } catch (error) {
        this.isConnecting = false;
        reject(error);
      }
    });
  }

  /**
   * Disconnect from WebSocket server
   */
  disconnect(): void {
    if (this.ws) {
      this.reconnectAttempts = this.maxReconnectAttempts; // Prevent reconnection
      this.ws.close();
      this.ws = null;
      this.notifyConnectionListeners(false);
    }
  }

  /**
   * Send a message through WebSocket
   */
  send(type: WebSocketMessageType, payload: MessagePayload): boolean {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      console.warn('WebSocket not connected, cannot send message');
      return false;
    }

    try {
      const message: WebSocketMessage = { type, payload };
      this.ws.send(JSON.stringify(message));
      return true;
    } catch (error) {
      console.error('Failed to send WebSocket message:', error);
      return false;
    }
  }

  /**
   * Register a handler for a specific message type
   */
  on(type: string, handler: MessageHandler): void {
    if (!this.handlers.has(type)) {
      this.handlers.set(type, new Set());
    }
    this.handlers.get(type)!.add(handler);
  }

  /**
   * Unregister a handler for a specific message type
   */
  off(type: string, handler: MessageHandler): void {
    const handlers = this.handlers.get(type);
    if (handlers) {
      handlers.delete(handler);
      if (handlers.size === 0) {
        this.handlers.delete(type);
      }
    }
  }

  /**
   * Listen to connection status changes
   */
  onConnectionChange(listener: (connected: boolean) => void): () => void {
    this.connectionListeners.add(listener);
    
    // Return unsubscribe function
    return () => {
      this.connectionListeners.delete(listener);
    };
  }

  /**
   * Check if WebSocket is connected
   */
  isConnected(): boolean {
    return this.ws?.readyState === WebSocket.OPEN;
  }

  /**
   * Send a chat message
   */
  sendChatMessage(conversationId: number, content: string): boolean {
    return this.send('chat:send', { conversation_id: conversationId, content });
  }

  /**
   * Send typing indicator
   */
  sendTypingIndicator(conversationId: number): boolean {
    return this.send('chat:typing', { conversation_id: conversationId });
  }

  /**
   * Handle incoming WebSocket message
   */
  private handleMessage(message: WebSocketMessage): void {
    const handlers = this.handlers.get(message.type);
    if (handlers) {
      handlers.forEach(handler => {
        try {
          handler(message);
        } catch (error) {
          console.error(`Error in handler for ${message.type}:`, error);
        }
      });
    }
  }

  /**
   * Attempt to reconnect with exponential backoff
   */
  private attemptReconnect(): void {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.log('Max reconnection attempts reached');
      return;
    }

    if (!this.token) {
      console.log('No token available for reconnection');
      return;
    }

    this.reconnectAttempts++;
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);

    console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

    setTimeout(() => {
      this.connect(this.token!)
        .catch(error => {
          console.error('Reconnection failed:', error);
        });
    }, delay);
  }

  /**
   * Notify all connection listeners of status change
   */
  private notifyConnectionListeners(connected: boolean): void {
    this.connectionListeners.forEach(listener => {
      try {
        listener(connected);
      } catch (error) {
        console.error('Error in connection listener:', error);
      }
    });
  }
}

// Export singleton instance
export const websocketService = new WebSocketService();

export default websocketService;
