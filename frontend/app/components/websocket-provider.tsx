'use client';

import { useEffect, useState } from 'react';
import { useWebSocket } from '../lib/use-websocket';
import { apiClient } from '../lib/api-client';
import websocketService from '../lib/websocket-service';

/**
 * WebSocket Provider Component
 * Manages WebSocket connection for the entire app
 */
export function WebSocketProvider({ 
  children
}: { 
  children: React.ReactNode;
}) {
  const [token, setToken] = useState<string | null>(null);

  // Get token from API client (localStorage)
  useEffect(() => {
    const authToken = apiClient.getToken();
    setToken(authToken);
  }, []);

  const { connected } = useWebSocket(token);

  useEffect(() => {
    if (connected) {
      console.log('✓ WebSocket connected and ready');
    } else if (token) {
      console.log('WebSocket disconnected (have token but not connected)');
    }
  }, [connected, token]);

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
