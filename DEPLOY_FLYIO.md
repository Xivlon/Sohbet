# Fly.io Deployment Guide - PostgreSQL Migration

This guide covers deploying the Sohbet backend to Fly.io with Neon PostgreSQL.

## Architecture

```
Vercel Frontend
    ↓
Fly.io Backend (C++ with PostgreSQL)
    ↓
Neon PostgreSQL (Managed Database)
```

## Prerequisites

- Fly.io CLI installed (`brew install flyctl` or visit https://fly.io/docs/hands-on/install-flyctl/)
- Fly.io account and logged in (`fly auth login`)
- Neon PostgreSQL database provisioned

## Step 1: Set Environment Secrets

Set your database connection string and JWT secret:

```bash
# Set DATABASE_URL (your Neon connection string)
fly secrets set DATABASE_URL="postgresql://neondb_owner:npg_Kdx0CAsargy5@ep-frosty-term-ag23z2o1-pooler.c-2.eu-central-1.aws.neon.tech/neondb?sslmode=require&channel_binding=require" -a sohbet-uezxqq

# Set JWT secret (generate a secure random string)
fly secrets set SOHBET_JWT_SECRET="vbvCE8RHT9pDvAUby0Gt6/GXHjdZWcsPmaapnm8YBhx0gB1YIJS4lEPnHiYFseNd" -a sohbet-uezxqq

# Optional: Set CORS origin if different from default
fly secrets set CORS_ORIGIN="https://your-vercel-app.vercel.app" -a sohbet-uezxqq
```

## Step 2: Verify Secrets

```bash
fly secrets list -a sohbet-uezxqq
```

Expected output:
```
NAME                 DIGEST                  CREATED AT
DATABASE_URL         xxxxxxxxxxxxx           X minutes ago
SOHBET_JWT_SECRET    xxxxxxxxxxxxx           X minutes ago
```

## Step 3: Deploy to Fly.io

From the project root directory:

```bash
# Build and deploy
fly deploy -c backend/fly.toml

# Monitor deployment
fly logs -a sohbet-uezxqq
```

## Step 4: Run Database Migrations

The migrations will run automatically on server startup when it connects to the database for the first time.

Monitor the logs to ensure migrations complete:

```bash
fly logs -a sohbet-uezxqq | grep -i migration
```

Expected output:
```
Loaded migration 0: create_users
Loaded migration 1: social_features
...
Migration 0 applied successfully
Migration 1 applied successfully
...
```

## Step 5: Verify Deployment

Test the API endpoints:

```bash
# Check server status
curl https://sohbet-uezxqq.fly.dev/api/status

# Expected response:
# {"status":"ok","message":"Academic Social Server is running","version":"0.3.0-academic"}
```

## Important Notes

### Database Connection
- The app now connects to **Neon PostgreSQL** instead of local SQLite
- No persistent storage volumes are needed (removed from fly.toml)
- Connection pooling is handled by Neon's pooler endpoint

### Migration from SQLite
- All 7 migration files have been converted to PostgreSQL syntax
- If you had existing data in SQLite, you'll need to migrate it manually
- The schema is now optimized for PostgreSQL (BIGSERIAL, TIMESTAMP, native BOOLEAN)

### Environment Variables
All required environment variables:
- `DATABASE_URL` - **REQUIRED** - PostgreSQL connection string
- `SOHBET_JWT_SECRET` - **REQUIRED** - JWT signing secret
- `CORS_ORIGIN` - Optional (defaults to localhost)
- `PORT` - Optional (defaults to 8080)
- `WS_PORT` - Optional (defaults to 8081)

### Frontend Configuration
Ensure your Vercel frontend has these environment variables set:

```
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
NEXT_PUBLIC_ENABLE_WEBSOCKET=true
```

## Troubleshooting

### View Logs
```bash
fly logs -a sohbet-uezxqq
```

### SSH into Container
```bash
fly ssh console -a sohbet-uezxqq
```

### Check Machine Status
```bash
fly status -a sohbet-uezxqq
```

### Database Connection Issues

If you see connection errors:
1. Verify DATABASE_URL secret is set correctly
2. Check Neon dashboard for database status
3. Ensure IP allowlist includes Fly.io IPs (if configured)

### Rollback Deployment
```bash
fly releases -a sohbet-uezxqq
fly rollback -a sohbet-uezxqq <version>
```

## Performance Considerations

### Neon Connection Pooling
The connection string uses Neon's pooler endpoint:
- Format: `@ep-xxx-pooler.c-2.eu-central-1.aws.neon.tech`
- Provides connection pooling out of the box
- Optimized for serverless/container environments

### Machine Specifications
Current configuration:
- 1 CPU (shared)
- 1GB RAM
- Auto-start/stop enabled

For higher traffic, consider:
```bash
fly scale vm shared-cpu-2x --memory 2048 -a sohbet-uezxqq
```

## Security Checklist

- ✅ DATABASE_URL stored as Fly.io secret (not in code)
- ✅ JWT_SECRET stored as Fly.io secret
- ✅ TLS enabled for HTTPS/WSS endpoints
- ✅ Connection uses SSL (sslmode=require)
- ✅ Parameterized queries prevent SQL injection
- ✅ Password hashing with bcrypt

## Next Steps

1. Set up monitoring (Fly.io metrics or external APM)
2. Configure alerting for downtime
3. Set up database backups (Neon provides automatic backups)
4. Consider adding a CDN for static assets
5. Implement rate limiting at the edge (Fly.io or Cloudflare)
