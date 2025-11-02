# IMMEDIATE FIX FOR 404 ERROR

## Problem
Vercel deployment shows **404: NOT_FOUND** error despite successful build.

## Root Cause
The repository is a **monorepo** with:
- Next.js frontend in `frontend/` subdirectory
- C++ backend in root and `backend/` directory

Vercel needs to be told to build from the `frontend/` directory, not the repository root.

## The Fix (Takes 2 Minutes)

### Step 1: Go to Vercel Dashboard
1. Open https://vercel.com/dashboard
2. Select your Sohbet project
3. Click "Settings" (top navigation)

### Step 2: Set Root Directory
1. In the left sidebar, click "General"
2. Scroll to "Build & Development Settings"
3. Find **Root Directory**
4. Click "Edit"
5. Enter: `frontend`
6. Click "Save"

### Step 3: Redeploy
1. Go to "Deployments" tab
2. Click the three dots (`...`) on the latest deployment
3. Select "Redeploy"
4. Click "Redeploy" to confirm

### Step 4: Verify
1. Wait for deployment to complete (~2 minutes)
2. Click on the deployment URL
3. You should see the Sohbet login page (no more 404!)

## Why This Works

**Before Fix:**
- Vercel looks in repository root for Next.js app
- Finds C++ backend instead
- Can't build properly → 404 error
- Reports wrong deployment size

**After Fix:**
- Vercel looks in `frontend/` directory
- Finds Next.js app
- Builds correctly → App loads!
- Reports correct deployment size (~11MB)

## Configuration Summary

### What You Set in Vercel Dashboard:
- **Root Directory**: `frontend` ← **CRITICAL!**
- **Framework Preset**: Next.js (auto-detects)
- **Build Command**: `npm run build` (auto-detects)
- **Output Directory**: `.next` (auto-detects)
- **Install Command**: `npm ci` (auto-detects)

### What's in vercel.json:
```json
{
  "$schema": "https://openapi.vercel.sh/vercel.json",
  "env": {
    "NEXT_PUBLIC_API_URL": "https://sohbet-uezxqq.fly.dev",
    "NEXT_PUBLIC_WS_URL": "wss://sohbet-uezxqq.fly.dev:8081"
  }
}
```

**Note**: Environment variables are automatically applied from `vercel.json`. You don't need to set them manually unless you want different values.

## Troubleshooting

### Still Getting 404?
- Double-check Root Directory is set to `frontend` (not `./frontend` or `/frontend`, just `frontend`)
- Make sure you clicked "Save" after setting Root Directory
- Trigger a fresh deployment after saving settings

### Build Fails?
- Check deployment logs for specific errors
- Verify Framework Preset is "Next.js"
- Ensure all build commands are auto-detected or empty

### Environment Variables Not Working?
- Variables must start with `NEXT_PUBLIC_` to be available in the browser
- Redeploy after changing environment variables
- Clear browser cache and hard refresh (Ctrl+Shift+R)

## What Changed in This Fix

We fixed the invalid `vercel.json` configuration:
- ❌ **REMOVED**: `buildCommand`, `outputDirectory`, `installCommand` (not valid in vercel.json)
- ✅ **KEPT**: Environment variables (valid configuration)
- ✅ **ADDED**: `frontend/vercel.json` for frontend-specific settings

These build-related settings must be configured in the Vercel Dashboard, not in vercel.json.

## For More Information

- [VERCEL_README.md](VERCEL_README.md) - Quick start guide
- [VERCEL_DEPLOYMENT.md](VERCEL_DEPLOYMENT.md) - Detailed deployment guide
- [VERCEL_CHECKLIST.md](VERCEL_CHECKLIST.md) - Complete deployment checklist

## Quick Deploy Button

For future deployments (after fixing Root Directory), use this:

[![Deploy with Vercel](https://vercel.com/button)](https://vercel.com/new/clone?repository-url=https://github.com/Xivlon/Sohbet&root-directory=frontend)

**Note**: The button includes `root-directory=frontend` parameter to automatically configure new deployments correctly.
