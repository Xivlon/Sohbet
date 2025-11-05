import type { NextConfig } from "next";
import createNextIntlPlugin from 'next-intl/plugin';

const withNextIntl = createNextIntlPlugin('./i18n.ts');

const nextConfig: NextConfig = {
  /* config options here */

  // Disable source maps in production to avoid 404 errors
  productionBrowserSourceMaps: false,

  // React optimization
  reactStrictMode: true,

  // Handle browser extension source map requests gracefully
  async headers() {
    return [
      {
        // Apply to all routes
        source: '/:path*',
        headers: [
          {
            // Prevent browser from showing source map errors for extension scripts
            key: 'X-SourceMap',
            value: 'none',
          },
          {
            // Security header
            key: 'X-Content-Type-Options',
            value: 'nosniff',
          },
        ],
      },
    ];
  },
  async rewrites() {
    return {
      // Handle browser extension source map requests
      beforeFiles: [
        {
          source: '/injection-:scriptname.js.map',
          destination: '/api/source-map-handler',
        },
        {
          source: '/%3Canonymous%20code%3E',
          destination: '/api/source-map-handler',
        },
      ],
    };
  },

  // Webpack configuration for additional source map control
  webpack: (config, { dev, isServer }) => {
    if (!dev && !isServer) {
      // Disable source maps in production client builds
      config.devtool = false;
    }
    return config;
  },
};

export default withNextIntl(nextConfig);
