#!/bin/bash
# Complete PostgreSQL Deployment Script for Fly.io
# Run this script from the repository root directory

set -e  # Exit on any error

echo "========================================="
echo "Sohbet PostgreSQL Deployment to Fly.io"
echo "========================================="
echo ""

# Configuration
APP_NAME="sohbet-uezxqq"
DATABASE_URL="postgresql://neondb_owner:npg_Kdx0CAsargy5@ep-frosty-term-ag23z2o1-pooler.c-2.eu-central-1.aws.neon.tech/neondb?sslmode=require&channel_binding=require"
JWT_SECRET="vbvCE8RHT9pDvAUby0Gt6/GXHjdZWcsPmaapnm8YBhx0gB1YIJS4lEPnHiYFseNd"

echo "Step 1: Checking Fly.io CLI..."
if ! command -v fly &> /dev/null; then
    echo "ERROR: Fly.io CLI not found. Please install it first:"
    echo "  brew install flyctl  (macOS)"
    echo "  or visit: https://fly.io/docs/hands-on/install-flyctl/"
    exit 1
fi
echo "✓ Fly.io CLI found"
echo ""

echo "Step 2: Verifying authentication..."
fly auth whoami -a $APP_NAME || {
    echo "ERROR: Not authenticated. Run: fly auth login"
    exit 1
}
echo "✓ Authenticated"
echo ""

echo "Step 3: Setting secrets..."
echo "  - Setting DATABASE_URL..."
fly secrets set DATABASE_URL="$DATABASE_URL" -a $APP_NAME
echo "  - Setting SOHBET_JWT_SECRET..."
fly secrets set SOHBET_JWT_SECRET="$JWT_SECRET" -a $APP_NAME
echo "✓ Secrets configured"
echo ""

echo "Step 4: Listing current machines..."
fly machines list -a $APP_NAME
echo ""

read -p "Do you want to destroy old machines? (y/N) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Destroying old machines..."
    fly machines list -a $APP_NAME --json | jq -r '.[].id' | while read machine_id; do
        echo "  - Destroying machine: $machine_id"
        fly machine destroy $machine_id -a $APP_NAME --force || true
    done
    echo "✓ Old machines destroyed"
else
    echo "Skipping machine cleanup"
fi
echo ""

echo "Step 5: Listing volumes..."
fly volumes list -a $APP_NAME
echo ""

read -p "Do you want to destroy old volumes? (y/N) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Destroying old volumes..."
    fly volumes list -a $APP_NAME --json | jq -r '.[].id' | while read volume_id; do
        echo "  - Destroying volume: $volume_id"
        fly volumes destroy $volume_id -a $APP_NAME --yes || true
    done
    echo "✓ Old volumes destroyed"
else
    echo "Skipping volume cleanup"
fi
echo ""

echo "Step 6: Deploying application..."
echo "This will:"
echo "  - Build Docker image with PostgreSQL libraries"
echo "  - Deploy to Fly.io"
echo "  - Run PostgreSQL migrations"
echo ""
fly deploy -c backend/fly.toml -a $APP_NAME --no-cache --build-arg BUILDKIT_INLINE_CACHE=0

echo ""
echo "Step 7: Checking deployment status..."
fly status -a $APP_NAME
echo ""

echo "Step 8: Viewing logs (last 100 lines)..."
fly logs -a $APP_NAME --tail=100
echo ""

echo "========================================="
echo "Deployment Complete!"
echo "========================================="
echo ""
echo "Next steps:"
echo "1. Check logs: fly logs -a $APP_NAME"
echo "2. Test API: curl https://sohbet-uezxqq.fly.dev/api/status"
echo "3. Monitor: https://fly.io/apps/$APP_NAME/monitoring"
echo ""
