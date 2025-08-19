#!/bin/bash

# Navigate to the server directory and start the server
cd server
npm install
npm run dev &

# Navigate to the client directory and start the client
cd ../client
npm install
npm run dev &

# Wait for both processes to finish
wait

echo "Development environment is up and running!"