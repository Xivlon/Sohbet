import { NextResponse } from 'next/server';

/**
 * Handler for browser extension source map requests
 * This prevents 404 errors in the console when browser extensions
 * (like security extensions) inject scripts and try to load source maps
 */
export async function GET() {
  // Return an empty source map structure to satisfy the request
  const emptySourceMap = {
    version: 3,
    sources: [],
    names: [],
    mappings: '',
    file: 'browser-extension.js'
  };

  return NextResponse.json(emptySourceMap, {
    status: 200,
    headers: {
      'Content-Type': 'application/json',
      'Cache-Control': 'public, max-age=31536000, immutable',
    },
  });
}
