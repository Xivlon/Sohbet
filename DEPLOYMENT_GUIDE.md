# Deployment Guide for Sohbet

This guide provides instructions for deploying Sohbet in production environments, including the necessary environment variable configurations.

## Environment Variables

### Frontend (Vercel/Next.js)

Create a `.env.local` file in the `frontend/` directory with the following variables:

```env
# Production Backend URLs - Update with your actual deployment domain
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev
```

**Important Notes:**
- `NEXT_PUBLIC_API_URL`: The base URL for your backend API (must use `https://` for production)
- `NEXT_PUBLIC_WS_URL`: The WebSocket URL (must use `wss://` for secure connections on HTTPS sites)
- Do NOT commit `.env.local` to version control (it's in `.gitignore`)
- For local development, you can use `http://localhost:8080` and `ws://localhost:8081`

#### Vercel Deployment

In your Vercel project settings, add these environment variables:

```
NEXT_PUBLIC_API_URL=https://your-backend-domain.fly.dev
NEXT_PUBLIC_WS_URL=wss://your-backend-domain.fly.dev
```

### Backend (Fly.io/C++)

Set the following environment variables on your backend deployment:

```bash
# On Fly.io
fly secrets set SOHBET_JWT_SECRET=your-secret-key-here
fly secrets set CORS_ORIGIN=https://sohbet-henna.vercel.app
fly secrets set PORT=8080
fly secrets set WS_PORT=8081
```

**Environment Variables:**
- `SOHBET_JWT_SECRET`: Secret key for JWT token generation (required)
- `CORS_ORIGIN`: Allowed origin for CORS requests (e.g., your frontend URL). Use `*` for development to allow all origins
- `PORT`: HTTP server port (default: 8080)
- `WS_PORT`: WebSocket server port (default: 8081)

## Security Considerations

### Protocol Mismatch Prevention

The frontend now includes validation to prevent insecure WebSocket connections (`ws://`) from HTTPS pages. If you attempt to connect to a non-secure WebSocket from an HTTPS page, you'll see an error message instructing you to configure the `NEXT_PUBLIC_WS_URL` environment variable.

### CORS Configuration

The backend server's CORS configuration is controlled by the `CORS_ORIGIN` environment variable:
- **Production**: Set to your frontend domain (e.g., `https://sohbet-henna.vercel.app`)
- **Development**: Can be set to `*` to allow all origins
- **Default**: If not set, defaults to `*` (not recommended for production)

## Deployment Checklist

### Frontend Deployment (Vercel)

- [ ] Set `NEXT_PUBLIC_API_URL` environment variable in Vercel
- [ ] Set `NEXT_PUBLIC_WS_URL` environment variable in Vercel
- [ ] Verify environment variables use `https://` and `wss://` protocols
- [ ] Deploy frontend
- [ ] Test API connectivity in browser console
- [ ] Verify WebSocket connection establishes successfully

### Backend Deployment (Fly.io)

- [ ] Set `SOHBET_JWT_SECRET` secret
- [ ] Set `CORS_ORIGIN` to frontend domain
- [ ] Optional: Set `PORT` and `WS_PORT` if using non-default ports
- [ ] Deploy backend
- [ ] Verify `/api/status` endpoint is accessible
- [ ] Check WebSocket port is accessible (not blocked by firewall)

## Testing Your Deployment

### 1. Test API Connection

Open your frontend in a browser and check the console:
```javascript
// Should show successful API request
fetch('https://your-backend.fly.dev/api/status')
  .then(r => r.json())
  .then(console.log)
```

### 2. Test WebSocket Connection

In the browser console:
```javascript
// Should establish connection without errors
const ws = new WebSocket('wss://your-backend.fly.dev/?token=test-token');
ws.onopen = () => console.log('✓ WebSocket connected');
ws.onerror = (e) => console.error('✗ WebSocket error:', e);
```

### 3. Verify Browser DevTools

- **Network tab**: Should show successful API requests (status 200/201)
- **Console**: No CORS errors
- **Console**: WebSocket connection shows as "connected"
- **Application tab**: Auth token stored in localStorage

## Troubleshooting

### CORS Errors

If you see CORS errors in the browser console:
1. Verify `CORS_ORIGIN` is set on the backend to your frontend URL
2. Ensure there's no trailing slash in the CORS_ORIGIN value
3. Restart the backend service after changing environment variables

### WebSocket Connection Failures

If WebSocket fails to connect:
1. Verify `NEXT_PUBLIC_WS_URL` uses `wss://` (not `ws://`) for HTTPS sites
2. Check that the WebSocket port (8081) is not blocked by firewall
3. Ensure the backend WebSocket server is running
4. Check browser console for specific error messages

### API 404 Errors

If API requests return 404:
1. Verify `NEXT_PUBLIC_API_URL` is correctly set
2. Check that the URL doesn't have duplicate slashes or trailing slashes
3. Ensure the backend is deployed and accessible
4. Test the `/api/status` endpoint directly in a browser

### Missing Environment Variables

If you see "API base URL not configured" errors:
1. Ensure environment variables are set in your deployment platform
2. For Vercel, environment variables must start with `NEXT_PUBLIC_` to be accessible in the browser
3. Rebuild and redeploy after setting environment variables

## Local Development

For local development, create `frontend/.env.local`:

```env
NEXT_PUBLIC_API_URL=http://localhost:8080
NEXT_PUBLIC_WS_URL=ws://localhost:8081
```

Then run:
```bash
# Terminal 1: Backend
cd /path/to/Sohbet
mkdir build && cd build
cmake ..
make
./sohbet

# Terminal 2: Frontend
cd /path/to/Sohbet/frontend
npm install
npm run dev
```

Access the application at `http://localhost:5000`

## Additional Resources

- [Vercel Environment Variables](https://vercel.com/docs/environment-variables)
- [Fly.io Secrets Management](https://fly.io/docs/reference/secrets/)
- [Next.js Environment Variables](https://nextjs.org/docs/basic-features/environment-variables)
