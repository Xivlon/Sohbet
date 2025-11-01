import { NextResponse } from 'next/server';
import type { NextRequest } from 'next/server';

/**
 * Middleware to handle browser extension source map requests
 * This prevents 404 errors in the console when browser extensions inject scripts
 */
export function middleware(request: NextRequest) {
  const { pathname } = request.nextUrl;
  
  // Check if this is a source map request that likely comes from a browser extension
  if (pathname.endsWith('.js.map')) {
    // List of known browser extension patterns
    const extensionPatterns = [
      'injection-',
      'content-script',
      'extension-',
      'chrome-extension',
      'moz-extension',
    ];
    
    // Check if the path matches any extension pattern
    const isExtensionSourceMap = extensionPatterns.some(pattern => 
      pathname.includes(pattern)
    );
    
    // Also check for encoded anonymous code paths
    const isAnonymousCode = pathname.includes('%3Canonymous') || 
                           pathname.includes('<anonymous');
    
    if (isExtensionSourceMap || isAnonymousCode) {
      // Return an empty source map
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
          'Cache-Control': 'public, max-age=86400, immutable',
        },
      });
    }
  }
  
  return NextResponse.next();
}

export const config = {
  matcher: [
    /*
     * Match all request paths except for the ones starting with:
     * - _next/static (static files)
     * - _next/image (image optimization files)
     * - favicon.ico (favicon file)
     */
    '/((?!_next/static|_next/image|favicon.ico).*)',
  ],
};
