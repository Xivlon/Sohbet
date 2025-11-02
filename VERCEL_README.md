# Vercel Deployment - IMPORTANT

## ⚠️ Critical Configuration Required

This repository is a **monorepo** with the Next.js frontend located in the `frontend/` subdirectory.

### Required Vercel Project Settings

To successfully deploy, you **MUST** configure these settings in your Vercel project dashboard:

1. **Go to Project Settings** → General → Build & Development Settings

2. **Set Root Directory**: `frontend`
   - This tells Vercel where to find the Next.js application
   - Without this, Vercel will look in the repo root and fail with 404

3. **Framework Preset**: Next.js (should auto-detect)

4. **Build Settings** (should auto-detect when Root Directory is set):
   - Build Command: `npm run build` (or leave empty)
   - Output Directory: `.next` (or leave empty)
   - Install Command: `npm ci` (or leave empty)

### Environment Variables

The following environment variables are configured in the root `/vercel.json` and will be automatically set:

```
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

You can override these in Vercel project settings if needed.

## Common Issues

### 404: NOT_FOUND Error

**Symptom**: Deployment succeeds but accessing the site shows "404: NOT_FOUND"

**Cause**: Root Directory is not set to `frontend`

**Fix**: 
1. Go to Vercel Project Settings → General
2. Set Root Directory to `frontend`
3. Save and redeploy

### Build Fails - "Could not find package.json"

**Cause**: Root Directory is not set correctly

**Fix**: Set Root Directory to `frontend` in project settings

## Deployment Checklist

Before deploying to Vercel:

- [ ] Set Root Directory to `frontend` in Vercel project settings
- [ ] Verify Framework Preset is "Next.js"
- [ ] Check environment variables are set (automatic from vercel.json)
- [ ] Trigger deployment
- [ ] Verify deployment succeeds and site loads correctly

## Testing Locally

To verify the build works before deploying:

```bash
cd frontend
npm ci
npm run build
npm start
```

The app should build successfully and run on http://localhost:5000

## Documentation

For more details, see:
- [VERCEL_DEPLOYMENT.md](VERCEL_DEPLOYMENT.md) - Complete deployment guide
- [frontend/README.md](frontend/README.md) - Frontend-specific documentation

## Quick Deploy Button

Once configured correctly, you can use this button for future deployments:

[![Deploy with Vercel](https://vercel.com/button)](https://vercel.com/new/clone?repository-url=https://github.com/Xivlon/Sohbet&root-directory=frontend)

**Note**: The button includes `root-directory=frontend` parameter to auto-configure the Root Directory setting.
