/**
 * Debug utility for configurable logging
 * Logs are enabled in development mode or when NEXT_PUBLIC_DEBUG_API is set to 'true'
 */

export const isDebugEnabled = (): boolean => {
  return typeof process !== 'undefined' && 
    (process.env.NODE_ENV === 'development' || process.env.NEXT_PUBLIC_DEBUG_API === 'true');
};

export const debugLog = (...args: unknown[]): void => {
  if (isDebugEnabled()) {
    console.log(...args);
  }
};
