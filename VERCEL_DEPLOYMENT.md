# Vercel Deployment Configuration

This document explains how to deploy the Sohbet frontend to Vercel from this monorepo.

## Quick Setup

### 1. Import Project to Vercel

1. Go to [Vercel](https://vercel.com) and click "Add New Project"
2. Import your GitHub repository
3. Vercel will detect it as a monorepo

### 2. Configure Project Settings

**IMPORTANT:** Set these in your Vercel project settings:

- **Framework Preset**: Next.js
- **Root Directory**: `frontend` ← **This is critical!**
- **Build Command**: Leave empty (auto-detected) or use `npm run build`
- **Output Directory**: Leave empty (auto-detected) or use `.next`
- **Install Command**: Leave empty (auto-detected) or use `npm ci`

### 3. Environment Variables (Optional)

The `vercel.json` file already includes default environment variables:

```
NEXT_PUBLIC_API_URL=https://sohbet-uezxqq.fly.dev
NEXT_PUBLIC_WS_URL=wss://sohbet-uezxqq.fly.dev:8081
```

These will be used automatically. **You only need to add environment variables in Vercel project settings if you want to override the defaults** or point to a different backend.

### 4. Deploy

Click "Deploy" and Vercel will:
- Detect Next.js in the `frontend` directory
- Run `npm ci` to install dependencies
- Run `npm run build` to build the app
- Deploy the `.next` output directory

## What Changed?

The previous `vercel.json` configuration used invalid fields that are not part of the Vercel configuration schema:
- ❌ `buildCommand`, `outputDirectory`, `installCommand` are NOT valid in `vercel.json`
- ✅ These are project settings configured in the Vercel dashboard
- ✅ The new `vercel.json` only contains valid configuration options

## Files

- **`vercel.json`**: Contains environment variables and minimal config
- **`.vercelignore`**: Excludes backend files and build artifacts from deployment
- **`frontend/`**: The Next.js application root directory

## Troubleshooting

### 404: NOT_FOUND Error

**Error**: Deployment shows "404: NOT_FOUND Code: NOT_FOUND"

**Root Cause**: Vercel is trying to deploy from the wrong directory or can't find the Next.js application.

**Solution**:
1. **Set Root Directory in Vercel Dashboard**:
   - Go to your Vercel project settings
   - Navigate to "General" → "Build & Development Settings"
   - Set **Root Directory** to: `frontend`
   - Click "Save"
   - Trigger a new deployment

2. **Verify Framework Detection**:
   - Make sure **Framework Preset** is set to "Next.js"
   - If not detected automatically, select it manually

3. **Check Build Settings**:
   - Build Command: Should be auto-detected as `npm run build` or leave empty
   - Output Directory: Should be auto-detected as `.next` or leave empty
   - Install Command: Should be auto-detected as `npm ci` or leave empty

**Why This Happens**:
- The repository is a monorepo with the Next.js app in `frontend/` subdirectory
- Without the Root Directory setting, Vercel looks in the repository root
- The root has a C++ backend, not a Next.js app
- Vercel can't find the Next.js application and returns 404

### Build Fails

**Error**: "Could not find package.json"
- **Solution**: Make sure "Root Directory" is set to `frontend`

**Error**: "Command not found: npm"
- **Solution**: Vercel should auto-detect Node.js. If not, make sure Framework Preset is "Next.js"

### Deployment Works But App Doesn't Load

- Check environment variables are set correctly
- Verify `NEXT_PUBLIC_API_URL` and `NEXT_PUBLIC_WS_URL` start with `NEXT_PUBLIC_`
- Check browser console for errors

### Deployment Size / "Packet Size" Discrepancy

**Issue**: Vercel may report an incorrect deployment size or "packet size" that doesn't match the actual build output.

**Explanation**: 
- This occurs when Vercel attempts to build from the wrong directory
- If the Root Directory is not set to `frontend`, Vercel may try to package the entire monorepo including the C++ backend
- The reported size will be much smaller than expected because Vercel can't properly process the build

**Solution**:
1. Set Root Directory to `frontend` in Vercel Dashboard
2. Verify that the build completes successfully (check deployment logs)
3. After a successful build, Vercel should report a size of approximately 10-15MB for the Next.js build
4. The deployment should include only the `.next` directory and necessary dependencies

**Verification**:
- Local build size: `du -sh frontend/.next` should show ~11MB
- Vercel should report similar deployment size after successful build
- If sizes don't match, the Root Directory is likely not configured correctly

### Still Having Issues?

1. Check the Vercel deployment logs for specific error messages
2. Verify the build succeeds locally: `cd frontend && npm run build`
3. Make sure all dependencies are in `package.json` (not just `devDependencies`)
4. Check that `frontend/app/page.tsx` and `frontend/app/layout.tsx` exist
5. Ensure Root Directory is set to `frontend` in Vercel project settings

## References

- [Vercel Documentation](https://vercel.com/docs)
- [Next.js on Vercel](https://vercel.com/docs/frameworks/nextjs)
- [Monorepo Deployment](https://vercel.com/docs/monorepos)
