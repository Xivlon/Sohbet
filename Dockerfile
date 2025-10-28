# Build stage
FROM ubuntu:22.04 as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    build-essential \
    libsqlite3-dev \
    libssl-dev \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy source
WORKDIR /app
COPY . .

# Build backend
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libsqlite3-0 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary from builder
COPY --from=builder /app/build/sohbet /app/sohbet

# Create directory for database
RUN mkdir -p /app/data

WORKDIR /app

# Expose ports
EXPOSE 8080 8081

# Set JWT secret (must be provided at runtime)
ENV SOHBET_JWT_SECRET=${SOHBET_JWT_SECRET}

# Run server
CMD ["./sohbet"]
