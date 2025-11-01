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

### Build Fails

**Error**: "Could not find package.json"
- **Solution**: Make sure "Root Directory" is set to `frontend`

**Error**: "Command not found: npm"
- **Solution**: Vercel should auto-detect Node.js. If not, make sure Framework Preset is "Next.js"

### Deployment Works But App Doesn't Load

- Check environment variables are set correctly
- Verify `NEXT_PUBLIC_API_URL` and `NEXT_PUBLIC_WS_URL` start with `NEXT_PUBLIC_`
- Check browser console for errors

## References

- [Vercel Documentation](https://vercel.com/docs)
- [Next.js on Vercel](https://vercel.com/docs/frameworks/nextjs)
- [Monorepo Deployment](https://vercel.com/docs/monorepos)
