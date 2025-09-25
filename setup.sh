#!/bin/bash

# Sohbet Setup Script
# This script builds the C++ backend, tests endpoints, and provides troubleshooting information

set -e  # Exit on any error

echo "🚀 Sohbet Backend Setup and Startup Script"
echo "==========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to test endpoint with retry
test_endpoint() {
    local url=$1
    local expected_status=$2
    local max_retries=5
    local retry_count=0
    
    echo "Testing endpoint: $url"
    
    while [ $retry_count -lt $max_retries ]; do
        if curl -s -f "$url" > /dev/null 2>&1; then
            print_status "Endpoint $url is responding"
            return 0
        fi
        
        retry_count=$((retry_count + 1))
        echo "Retry $retry_count/$max_retries..."
        sleep 1
    done
    
    print_error "Endpoint $url failed after $max_retries attempts"
    return 1
}

# Check prerequisites
echo ""
echo "📋 Checking Prerequisites..."

if ! command_exists cmake; then
    print_error "CMake is required but not installed. Please install CMake 3.16+"
    exit 1
fi
print_status "CMake found"

if ! command_exists make; then
    print_error "Make is required but not installed. Please install build-essential"
    exit 1
fi
print_status "Make found"

if ! command_exists g++; then
    print_error "G++ compiler is required but not installed. Please install build-essential"
    exit 1
fi
print_status "G++ compiler found"

if ! command_exists curl; then
    print_error "curl is required for endpoint testing but not installed. Please install curl"
    exit 1
fi
print_status "curl found"

# Check if port 8080 is available
if lsof -Pi :8080 -sTCP:LISTEN -t >/dev/null 2>&1; then
    print_warning "Port 8080 is already in use. This may cause startup issues."
    print_info "Run 'lsof -i :8080' to see which process is using port 8080"
    print_info "You may need to stop the existing process or choose a different port"
fi

# Create build directory
echo ""
echo "🏗️  Building Backend..."

if [ ! -d "build" ]; then
    print_info "Creating build directory..."
    mkdir -p build
fi

cd build

# Configure with CMake
print_info "Configuring with CMake..."
if ! cmake .. > cmake_output.log 2>&1; then
    print_error "CMake configuration failed!"
    echo "CMake output:"
    cat cmake_output.log
    exit 1
fi
print_status "CMake configuration completed"

# Build with make
print_info "Building with make..."
if ! make > make_output.log 2>&1; then
    print_error "Build failed!"
    echo "Make output:"
    cat make_output.log
    exit 1
fi
print_status "Build completed successfully"

# Check if binary exists
if [ ! -f "sohbet" ]; then
    print_error "sohbet binary not found after build!"
    print_info "Check build logs above for errors"
    exit 1
fi

if [ ! -x "sohbet" ]; then
    print_error "sohbet binary is not executable!"
    chmod +x sohbet
fi

print_status "sohbet binary is ready"

cd ..

# Start server in background for testing
echo ""
echo "🌐 Starting Backend Server..."

print_info "Starting server on port 8080..."
cd build
./sohbet > server_output.log 2>&1 &
SERVER_PID=$!
cd ..

# Function to cleanup on exit
cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        print_info "Stopping server (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

# Set trap to cleanup on exit
trap cleanup EXIT

# Wait for server to start
print_info "Waiting for server to start..."
sleep 3

# Check if server process is still running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    print_error "Server process died unexpectedly!"
    echo "Server output:"
    cat build/server_output.log
    exit 1
fi

print_status "Server process is running (PID: $SERVER_PID)"

# Test endpoints
echo ""
echo "🧪 Testing API Endpoints..."

# Test /api/status endpoint
if test_endpoint "http://localhost:8080/api/status"; then
    RESPONSE=$(curl -s http://localhost:8080/api/status)
    print_status "GET /api/status response: $RESPONSE"
else
    print_error "Status endpoint test failed"
    echo "Server output:"
    cat build/server_output.log
    exit 1
fi

# Test /api/users/demo endpoint
if test_endpoint "http://localhost:8080/api/users/demo"; then
    RESPONSE=$(curl -s http://localhost:8080/api/users/demo)
    print_status "GET /api/users/demo response: $RESPONSE"
else
    print_error "Demo users endpoint test failed"
    exit 1
fi

# Test POST endpoint with a test user
print_info "Testing POST /api/users endpoint..."
RESPONSE=$(curl -s -X POST -H "Content-Type: application/json" \
    -d '{"username":"setup_test_user","email":"test@setup.edu","password":"TestPass123","university":"Setup University","department":"Test","enrollment_year":2023,"primary_language":"English"}' \
    http://localhost:8080/api/users)

if echo "$RESPONSE" | grep -q '"id"'; then
    print_status "POST /api/users test successful: $RESPONSE"
else
    print_warning "POST /api/users test may have issues: $RESPONSE"
fi

echo ""
print_status "🎉 Backend Setup Completed Successfully!"
echo ""
echo "Backend server is running on http://localhost:8080"
echo "Available endpoints:"
echo "  GET  http://localhost:8080/api/status"
echo "  GET  http://localhost:8080/api/users/demo"
echo "  POST http://localhost:8080/api/users"
echo "  POST http://localhost:8080/api/login"
echo ""
echo "Frontend connects directly to C++ backend: http://localhost:8080"
echo ""
echo "🔧 Troubleshooting Commands:"
echo "  - Check server status: curl http://localhost:8080/api/status"
echo "  - View server logs: tail -f build/server_output.log"
echo "  - Check port usage: lsof -i :8080"
echo "  - Stop server: kill $SERVER_PID"
echo ""
echo "Press Ctrl+C to stop the server and exit"

# Keep script running to maintain server
wait $SERVER_PID 2>/dev/null || true