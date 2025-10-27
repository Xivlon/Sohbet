'use client';

import { useEffect } from 'react';
import { useWebSocket, useWebSocketMessage, useChatWebSocket } from '../lib/use-websocket';

/**
 * WebSocket Provider Component
 * Manages WebSocket connection for the entire app
 */
export function WebSocketProvider({ 
  children, 
  token 
}: { 
  children: React.ReactNode;
  token: string | null;
}) {
  const { connected } = useWebSocket(token);

  useEffect(() => {
    if (connected) {
      console.log('✓ WebSocket connected and ready');
    } else {
      console.log('WebSocket disconnected');
    }
  }, [connected]);

  return <>{children}</>;
}

/**
 * Connection status indicator component
 */
export function ConnectionStatus({ className }: { className?: string }) {
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const unsubscribe = websocketService.onConnectionChange(setConnected);
    setConnected(websocketService.isConnected());
    return unsubscribe;
  }, []);

  if (!connected) {
    return (
      <div className={`flex items-center gap-2 text-sm text-yellow-600 ${className}`}>
        <div className="w-2 h-2 rounded-full bg-yellow-500 animate-pulse" />
        <span>Reconnecting...</span>
      </div>
    );
  }

  return (
    <div className={`flex items-center gap-2 text-sm text-green-600 ${className}`}>
      <div className="w-2 h-2 rounded-full bg-green-500" />
      <span>Connected</span>
    </div>
  );
}

// Import the service for ConnectionStatus component
import { useState } from 'react';
import websocketService from '../lib/websocket-service';
