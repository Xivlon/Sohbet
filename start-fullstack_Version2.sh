#!/bin/bash

# Sohbet Full Stack Startup Script
# This script starts both the C++ backend and React frontend

echo "🚀 Starting Sohbet Full Stack Application"
echo "========================================="

# Check if C++ backend is built
if [ ! -f "build/sohbet" ]; then
    echo "C++ backend not found. Building..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
    
    if [ ! -f "build/sohbet" ]; then
        echo "Failed to build C++ backend"
        exit 1
    fi
fi

# Check if frontend dependencies are installed
if [ ! -d "frontend/node_modules" ]; then
    echo "Frontend dependencies not found. Installing..."
    cd frontend
    npm install
    cd ..
fi

# Build the frontend
echo "Building frontend..."
cd frontend
if ! npm run build; then
    echo "Failed to build frontend"
    exit 1
fi
cd ..

echo "Prerequisites ready"
echo ""

# Start backend in background
echo "Starting C++ Backend Server on port 8080..."
cd build
./sohbet &
BACKEND_PID=$!
cd ..

# Wait a moment for backend to start
sleep 3

# Start frontend in production mode
echo "Starting React Frontend on port 5000..."
cd frontend
npm start &
FRONTEND_PID=$!
cd ..

echo ""
echo "Both servers started!"
echo "Frontend: http://0.0.0.0:5000"
echo "Backend API: http://0.0.0.0:8080/api/status"
echo ""
echo "Press Ctrl+C to stop all servers"

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "Stopping servers..."
    kill $BACKEND_PID $FRONTEND_PID 2>/dev/null
    echo "All servers stopped"
    exit 0
}

# Set trap to cleanup on Ctrl+C
trap cleanup SIGINT

# Wait for either process to finish
wait