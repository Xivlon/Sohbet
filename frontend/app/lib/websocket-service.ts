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
  | 'user:offline'
  | 'voice:join'
  | 'voice:leave'
  | 'voice:offer'
  | 'voice:answer'
  | 'voice:ice-candidate'
  | 'voice:mute'
  | 'voice:video-toggle'
  | 'voice:user-joined'
  | 'voice:user-left'
  | 'voice:participants'
  | 'voice:user-muted'
  | 'voice:user-video-toggled';

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
      console.log('🔌 WebSocket already connected');
      return Promise.resolve();
    }

    if (this.isConnecting) {
      console.log('🔌 WebSocket connection already in progress');
      return Promise.resolve();
    }

    this.token = token;
    this.isConnecting = true;

    return new Promise((resolve, reject) => {
      try {
        // Include token in URL as query parameter
        const wsUrl = `${this.url}/?token=${encodeURIComponent(token)}`;

        console.log('🔌 Attempting WebSocket connection...');
        console.log('   URL:', this.url);
        console.log('   Full URL:', wsUrl.replace(/token=[\w\.\-]+/, 'token=***'));
        console.log('   Protocol:', typeof window !== 'undefined' ? window.location.protocol : 'N/A');
        console.log('   Hostname:', typeof window !== 'undefined' ? window.location.hostname : 'N/A');

        // Check for insecure connection on HTTPS pages
        if (typeof window !== 'undefined' &&
            window.location.protocol === 'https:' &&
            wsUrl.startsWith('ws://')) {
          const error = new Error(
            'Cannot establish insecure WebSocket connection (ws://) from secure page (https://). ' +
            'Please configure NEXT_PUBLIC_WS_URL environment variable with a secure WebSocket URL (wss://)'
          );
          console.error('🔌 ❌', error.message);
          reject(error);
          return;
        }

        this.ws = new WebSocket(wsUrl);

        this.ws.onopen = () => {
          console.log('🔌 ✅ WebSocket connected successfully');
          console.log('   Ready state:', this.ws!.readyState);
          this.isConnecting = false;
          this.reconnectAttempts = 0;
          this.notifyConnectionListeners(true);
          resolve();
        };

        this.ws.onmessage = (event) => {
          try {
            const rawData = event.data;

            // Handle Blob data by converting to text first
            if (rawData instanceof Blob) {
              rawData.text().then((text: string) => {
                console.log('📨 Received Blob message:', text.substring(0, 100));
                this.processWebSocketData(text);
              }).catch((err: unknown) => {
                console.error('📨 ❌ Failed to convert Blob to text:', err);
              });
              return;
            }

            // Handle ArrayBuffer data
            if (rawData instanceof ArrayBuffer) {
              const text = new TextDecoder().decode(rawData);
              console.log('📨 Received ArrayBuffer message:', text.substring(0, 100));
              this.processWebSocketData(text);
              return;
            }

            // Handle string data (most common case)
            if (typeof rawData === 'string') {
              console.log('📨 Received message:', rawData.substring(0, 100));
              this.processWebSocketData(rawData);
              return;
            }

            console.error('📨 ❌ Unknown WebSocket data type:', typeof rawData);
          } catch (error) {
            console.error('📨 ❌ Error in WebSocket onmessage:', error);
          }
        };

        this.ws.onerror = (error) => {
          console.error('🔌 ❌ WebSocket error:', error);
          console.error('   Event:', error);
          if (this.ws) {
            console.error('   Ready state:', this.ws.readyState);
          }
          this.isConnecting = false;
          reject(error);
        };

        this.ws.onclose = (event) => {
          console.log('🔌 ❌ WebSocket closed');
          console.log('   Code:', event.code);
          console.log('   Reason:', event.reason);
          console.log('   Was clean:', event.wasClean);
          this.isConnecting = false;
          this.notifyConnectionListeners(false);
          this.attemptReconnect();
        };
      } catch (error) {
        console.error('🔌 ❌ Failed to create WebSocket:', error);
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
      console.warn('📤 ❌ WebSocket not connected, cannot send message');
      console.warn('   Type:', type);
      console.warn('   Ready state:', this.ws?.readyState);
      return false;
    }

    try {
      const message: WebSocketMessage = { type, payload };
      console.log('📤 Sending message:', type);
      console.log('   Payload:', JSON.stringify(payload).substring(0, 100));
      this.ws.send(JSON.stringify(message));
      return true;
    } catch (error) {
      console.error('📤 ❌ Failed to send WebSocket message:', error);
      console.error('   Type:', type);
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
    console.log('🔄 Processing message:', message.type);

    // Log WebRTC signaling messages with more detail
    if (message.type.startsWith('voice:')) {
      console.log('   🎤 WebRTC signaling -', message.type);
      if (message.type === 'voice:offer' || message.type === 'voice:answer') {
        console.log('      SDP received');
      } else if (message.type === 'voice:ice-candidate') {
        console.log('      ICE candidate received');
      } else if (message.type === 'voice:user-joined') {
        console.log('      User joined:', (message.payload as any).user_id);
      } else if (message.type === 'voice:user-left') {
        console.log('      User left:', (message.payload as any).user_id);
      } else if (message.type === 'voice:participants') {
        console.log('      Room participants:', JSON.stringify(message.payload).substring(0, 100));
      }
    }

    const handlers = this.handlers.get(message.type);
    if (handlers) {
      console.log(`   Calling ${handlers.size} handler(s) for ${message.type}`);
      handlers.forEach(handler => {
        try {
          handler(message);
        } catch (error) {
          console.error(`🔄 ❌ Error in handler for ${message.type}:`, error);
        }
      });
    } else {
      console.warn(`🔄 ⚠️ No handlers registered for ${message.type}`);
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

  /**
   * Process WebSocket data with robust error handling
   * Handles single messages, multiple concatenated messages, and malformed data
   */
  private processWebSocketData(rawData: string): void {
    if (!rawData || rawData.trim().length === 0) {
      return;
    }

    // First, try to parse as a single message (most common case)
    try {
      const message: WebSocketMessage = JSON.parse(rawData);
      this.handleMessage(message);
      return;
    } catch (error) {
      // If single parse fails, try to handle multiple messages or recover from errors
      // Check if this is the specific "unexpected character after JSON" error
      if (error instanceof SyntaxError && error.message.includes('after JSON data')) {
        // This indicates multiple concatenated messages
        try {
          const messages = this.parseMultipleMessages(rawData);
          if (messages.length > 0) {
            console.log(`Parsed ${messages.length} concatenated WebSocket messages`);
            messages.forEach(msg => this.handleMessage(msg));
            return;
          }
        } catch (recoveryError) {
          console.error('Failed to parse concatenated messages:', recoveryError);
        }
      }

      // If we still can't parse, log detailed error information
      console.error('Failed to parse WebSocket message:', error);
      console.error('Raw data (first 500 chars):', rawData.substring(0, 500));
      console.error('Data length:', rawData.length);

      // Try one more aggressive recovery attempt
      try {
        const messages = this.parseMultipleMessages(rawData);
        if (messages.length > 0) {
          console.warn(`Recovered ${messages.length} messages through aggressive parsing`);
          messages.forEach(msg => this.handleMessage(msg));
          return;
        }
      } catch (finalError) {
        console.error('All parsing attempts failed');
      }
    }
  }

  /**
   * Attempt to parse multiple concatenated JSON messages
   * This handles the case where the server incorrectly sends multiple JSON objects
   * in a single WebSocket frame (e.g., "}{" concatenation issue)
   */
  private parseMultipleMessages(rawData: string): WebSocketMessage[] {
    const messages: WebSocketMessage[] = [];

    // Try to find and parse all valid JSON objects in the string
    // Look for patterns like "}{"  which indicate concatenated objects
    let remaining = rawData.trim();
    let braceCount = 0;
    let startIndex = 0;
    let inString = false;
    let escapeNext = false;

    for (let i = 0; i < remaining.length; i++) {
      const char = remaining[i];

      // Handle string escaping to avoid counting braces inside strings
      if (escapeNext) {
        escapeNext = false;
        continue;
      }

      if (char === '\\') {
        escapeNext = true;
        continue;
      }

      if (char === '"') {
        inString = !inString;
        continue;
      }

      // Only count braces outside of strings
      if (!inString) {
        if (char === '{') {
          if (braceCount === 0) {
            startIndex = i;
          }
          braceCount++;
        } else if (char === '}') {
          braceCount--;

          // Found a complete JSON object
          if (braceCount === 0) {
            const jsonStr = remaining.substring(startIndex, i + 1);
            try {
              const parsed = JSON.parse(jsonStr);

              // Validate that it looks like a WebSocket message
              if (parsed && typeof parsed === 'object' && 'type' in parsed && 'payload' in parsed) {
                messages.push(parsed as WebSocketMessage);
              } else {
                console.warn('Parsed object does not match WebSocketMessage format');
              }
            } catch (e) {
              // Skip invalid JSON fragments
              const preview = jsonStr.length > 100 ? jsonStr.substring(0, 100) + '...' : jsonStr;
              console.warn('Skipping invalid JSON fragment:', preview);
            }

            // Reset for potential next message
            startIndex = i + 1;
          } else if (braceCount < 0) {
            // More closing braces than opening - reset
            console.warn('Brace mismatch detected, resetting parser');
            braceCount = 0;
            startIndex = i + 1;
          }
        }
      }
    }

    // Check if we have unclosed braces (malformed JSON)
    if (braceCount > 0) {
      console.warn(`Unclosed braces detected: ${braceCount} unmatched opening braces`);
    }

    return messages;
  }
}

// Export singleton instance
export const websocketService = new WebSocketService();

export default websocketService;
