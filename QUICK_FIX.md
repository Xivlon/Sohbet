# 🚀 Quick Fix: Vercel 404 Error

## The Problem
```
404: NOT_FOUND
Code: NOT_FOUND
```

## The Solution (2 Minutes)

### Step 1: Open Vercel Project Settings
Go to your Vercel project → Settings → General

### Step 2: Set Root Directory
Under "Build & Development Settings":
```
Root Directory: frontend
```

### Step 3: Save & Deploy
Click "Save" then redeploy your project.

**That's it!** ✅

---

## Why?

Your repo structure:
```
Sohbet/
├── backend/     ← C++ backend (not for Vercel)
└── frontend/    ← Next.js app (THIS is what Vercel needs)
```

Without the Root Directory setting, Vercel looks in the wrong place and can't find your Next.js app.

## Need More Help?

📚 **Full Guides:**
- [VERCEL_README.md](VERCEL_README.md) - Start here
- [VERCEL_CHECKLIST.md](VERCEL_CHECKLIST.md) - Step-by-step
- [VERCEL_FIX_SUMMARY.md](VERCEL_FIX_SUMMARY.md) - Complete explanation

## Still Not Working?

Check these:
1. ✅ Root Directory = `frontend` (exactly, no slashes)
2. ✅ Framework Preset = Next.js
3. ✅ Redeploy after changing settings
4. ✅ Check deployment logs for specific errors

---

**Environment Variables** (automatic):
- `NEXT_PUBLIC_API_URL` - Backend API
- `NEXT_PUBLIC_WS_URL` - WebSocket server

These are set in `vercel.json` and work automatically once Root Directory is correct.
