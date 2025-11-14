// Determine API URL based on environment
export const API_URL: string | undefined = process.env.NEXT_PUBLIC_API_URL ||
  (typeof window !== 'undefined' && process.env.NODE_ENV === 'production'
    ? 'https://sohbet-uezxqq.fly.dev'
    : undefined);

// Determine WebSocket URL based on environment
// If NEXT_PUBLIC_WS_URL is not set:
// - Production: uses same origin without port (wss://sohbet-uezxqq.fly.dev)
// - Development: uses localhost:8081
export const WS_URL: string | undefined = process.env.NEXT_PUBLIC_WS_URL ||
  (typeof window !== 'undefined' && process.env.NODE_ENV === 'production'
    ? `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.hostname}`
    : undefined);
