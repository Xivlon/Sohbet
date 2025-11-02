# Vercel Deployment Checklist

Use this checklist when deploying the Sohbet frontend to Vercel.

## Pre-Deployment

- [ ] Repository is pushed to GitHub
- [ ] All changes are committed
- [ ] Local build succeeds: `cd frontend && npm ci && npm run build`

## Vercel Project Setup (First Time Only)

- [ ] Go to [Vercel Dashboard](https://vercel.com/dashboard)
- [ ] Click "Add New Project"
- [ ] Import the Sohbet repository from GitHub
- [ ] Configure project settings (see below)

## Critical Project Settings

**⚠️ These settings are REQUIRED for deployment to work:**

### General Settings
- [ ] **Root Directory**: Set to `frontend`
  - Location: Project Settings → General → Build & Development Settings
  - This is the most important setting!

- [ ] **Framework Preset**: Next.js
  - Should auto-detect once Root Directory is set
  - If not, select manually

### Build & Development Settings
When Root Directory is set to `frontend`, these should auto-detect:

- [ ] **Build Command**: `npm run build` (or leave empty for auto-detect)
- [ ] **Output Directory**: `.next` (or leave empty for auto-detect)  
- [ ] **Install Command**: `npm ci` (or leave empty for auto-detect)

### Environment Variables
These are automatically configured in `vercel.json`:

- [ ] `NEXT_PUBLIC_API_URL`: Points to backend (https://sohbet-uezxqq.fly.dev)
- [ ] `NEXT_PUBLIC_WS_URL`: Points to WebSocket (wss://sohbet-uezxqq.fly.dev:8081)

**Optional**: Override in Vercel Project Settings → Environment Variables if needed

## Deploy

- [ ] Click "Deploy" button in Vercel
- [ ] Wait for build to complete (should take 1-2 minutes)
- [ ] Check deployment logs for any errors

## Post-Deployment Verification

- [ ] Visit the deployed URL (e.g., https://your-project.vercel.app)
- [ ] Verify the app loads (should show login screen)
- [ ] Check browser console for errors (F12)
- [ ] Test login functionality
- [ ] Verify API connection (check Network tab)
- [ ] Test WebSocket connection (look for ws:// connections)

## Troubleshooting

### Build Succeeds But Shows 404

**Cause**: Root Directory not set correctly

**Fix**:
1. Go to Project Settings → General
2. Set Root Directory to `frontend`
3. Save changes
4. Redeploy

### Build Fails - "Could not find package.json"

**Cause**: Root Directory not set

**Fix**: Set Root Directory to `frontend`

### Environment Variables Not Working

**Check**:
- Variables in Vercel must start with `NEXT_PUBLIC_` to be exposed to browser
- Redeploy after changing environment variables
- Clear browser cache

### Still Having Issues?

1. Check deployment logs in Vercel dashboard
2. Read [VERCEL_README.md](VERCEL_README.md)
3. Read [VERCEL_DEPLOYMENT.md](VERCEL_DEPLOYMENT.md)
4. Verify local build works: `cd frontend && npm run build`

## Production Checklist

Before going live:

- [ ] Update backend URLs if using custom domain
- [ ] Set up custom domain in Vercel (optional)
- [ ] Configure CORS on backend to allow your Vercel domain
- [ ] Enable HTTPS (automatic with Vercel)
- [ ] Test all features on production deployment
- [ ] Monitor deployment logs for errors

## Updating Deployment

When you push changes to GitHub:

- [ ] Vercel auto-deploys on push to main branch
- [ ] Check deployment status in Vercel dashboard
- [ ] Verify changes are live
- [ ] Test critical functionality

## Quick Reference

**Important Files:**
- `/vercel.json` - Root config with environment variables
- `/frontend/vercel.json` - Frontend-specific config
- `/.vercelignore` - Files to exclude from deployment

**Documentation:**
- [VERCEL_README.md](VERCEL_README.md) - Start here for deployment
- [VERCEL_DEPLOYMENT.md](VERCEL_DEPLOYMENT.md) - Detailed guide
- [README.md](README.md) - Main project documentation

**Vercel Resources:**
- [Vercel Dashboard](https://vercel.com/dashboard)
- [Vercel Docs - Next.js](https://vercel.com/docs/frameworks/nextjs)
- [Vercel Docs - Monorepos](https://vercel.com/docs/monorepos)
