import { NextResponse } from 'next/server';
import type { NextRequest } from 'next/server';
import createIntlMiddleware from 'next-intl/middleware';
import { locales } from './i18n';

// Create the i18n middleware
const intlMiddleware = createIntlMiddleware({
  locales,
  defaultLocale: 'tr',
  localeDetection: true,
  localePrefix: 'as-needed'
});

/**
 * Combined proxy to handle:
 * 1. Internationalization (i18n) routing
 * 2. Browser extension source map requests
 */
export function proxy(request: NextRequest) {
  const { pathname } = request.nextUrl;

  // Handle browser extension source map requests
  if (pathname.endsWith('.js.map')) {
    const extensionPatterns = [
      'injection-',
      'content-script',
      'extension-',
      'chrome-extension',
      'moz-extension',
    ];

    const isExtensionSourceMap = extensionPatterns.some(pattern =>
      pathname.includes(pattern)
    );

    const isAnonymousCode = pathname.includes('%3Canonymous') ||
                           pathname.includes('<anonymous');

    if (isExtensionSourceMap || isAnonymousCode) {
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

  // Handle i18n routing
  return intlMiddleware(request);
}

export const config = {
  matcher: [
    // Match all pathnames except for
    // - … if they start with `/api`, `/_next` or `/_vercel`
    // - … the ones containing a dot (e.g. `favicon.ico`)
    '/((?!api|_next|_vercel|.*\\..*).*)',
  ]
};
