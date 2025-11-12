# Quick PostgreSQL Deployment Commands

## ✅ Pre-flight Check

Everything is ready:
- ✅ Code migrated to PostgreSQL (BIGSERIAL, not AUTOINCREMENT)
- ✅ Dockerfile updated with libpq-dev and libpq5
- ✅ fly.toml updated (no SQLite mounts)
- ✅ All changes committed and pushed

## 🚀 Deployment Commands

Copy and paste these commands one by one:

### 1. Set Secrets

```bash
fly secrets set DATABASE_URL="postgresql://neondb_owner:npg_Kdx0CAsargy5@ep-frosty-term-ag23z2o1-pooler.c-2.eu-central-1.aws.neon.tech/neondb?sslmode=require&channel_binding=require" -a sohbet-uezxqq

fly secrets set SOHBET_JWT_SECRET="vbvCE8RHT9pDvAUby0Gt6/GXHjdZWcsPmaapnm8YBhx0gB1YIJS4lEPnHiYFseNd" -a sohbet-uezxqq
```

### 2. Clean Up Old Resources

```bash
# List machines
fly machines list -a sohbet-uezxqq

# Destroy old machine (replace ID with actual machine ID from above)
fly machine destroy 0802792f02e738 -a sohbet-uezxqq --force

# List volumes
fly volumes list -a sohbet-uezxqq

# Destroy old volume if any (replace ID with actual volume ID)
fly volumes destroy <volume-id> -a sohbet-uezxqq --yes
```

### 3. Deploy (Force Fresh Build)

```bash
cd /home/user/Sohbet

fly deploy -c backend/fly.toml -a sohbet-uezxqq --no-cache --local-only
```

**Important flags:**
- `--no-cache`: Force fresh Docker build (no cached layers)
- `--local-only`: Build from local directory (ensures PostgreSQL migrations are included)

### 4. Monitor Deployment

```bash
# Watch logs in real-time
fly logs -a sohbet-uezxqq

# Check for successful migration messages:
# - "Loaded migration 0: create_users"
# - "Migration 0 applied successfully"
# - "Server initialized successfully"
```

### 5. Verify Deployment

```bash
# Check machine status
fly status -a sohbet-uezxqq

# Test API endpoint
curl https://sohbet-uezxqq.fly.dev/api/status

# Expected response:
# {"status":"ok","message":"Academic Social Server is running","version":"0.3.0-academic"}
```

## 🐛 Troubleshooting

### If deployment fails with "AUTOINCREMENT" error:

This means it's using old migrations. Force rebuild:

```bash
fly deploy -c backend/fly.toml -a sohbet-uezxqq --no-cache --local-only --build-arg BUILDKIT_INLINE_CACHE=0
```

### If machine won't start:

```bash
# Check logs for errors
fly logs -a sohbet-uezxqq

# Common issues:
# 1. DATABASE_URL not set → Run Step 1 again
# 2. Can't connect to Neon → Check Neon dashboard
# 3. Old volume attached → Run Step 2 again
```

### View detailed machine info:

```bash
fly machine status <machine-id> -a sohbet-uezxqq
```

## 📊 Success Indicators

You'll know it worked when you see in logs:

```
Starting Sohbet Academic Social Backend v0.3.0-academic
Loaded migration 0: create_users
Loaded migration 1: social_features
Loaded migration 2: email_verification
Loaded migration 3: seed_turkish_organizations
Loaded migration 4: enhanced_features
Loaded migration 5: study_buddy_matching
Applying migration 0: create_users
Migration 0 applied successfully
...
Server initialized successfully
Academic Social Server starting on port 8080
Database: PostgreSQL (connection configured)
```

## 🎯 Architecture After Deployment

```
Vercel Frontend (your-app.vercel.app)
         ↓ HTTPS/WSS
Fly.io Backend (sohbet-uezxqq.fly.dev)
  - Port 8080: HTTP API
  - Port 8081: WebSocket
         ↓ PostgreSQL Protocol
Neon PostgreSQL (ep-frosty-term-ag23z2o1)
  - Managed database
  - Automatic backups
  - Connection pooling
```

## ⏱️ Deployment Time

- Setting secrets: ~10 seconds
- Destroying machines: ~30 seconds
- Building Docker image: ~5-10 minutes
- Deploying: ~1-2 minutes
- **Total: ~7-13 minutes**

## 🔐 Security Checklist

- ✅ DATABASE_URL stored as Fly.io secret (not in code)
- ✅ JWT_SECRET stored as Fly.io secret
- ✅ TLS enabled (HTTPS/WSS)
- ✅ SSL connection to database (sslmode=require)
- ✅ No SQLite files (using external PostgreSQL)

## 📝 Notes

- Your current branch `claude/explore-dedicated-sql-server-011CUzfr99MU6T53u2SKesfs` has all changes
- Fly.io will build from your local directory
- First deployment will run all 6 migrations
- Subsequent deployments won't re-run migrations (they're tracked in `schema_migrations` table)
