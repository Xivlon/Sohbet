# Build stage
FROM ubuntu:22.04 as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    build-essential \
    libpq-dev \
    postgresql-server-dev-all \
    libssl-dev \
    libcurl4-openssl-dev \
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
    libpq5 \
    libssl3 \
    libcurl4 \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary from builder
COPY --from=builder /app/build/sohbet /app/sohbet

# Copy migrations
COPY --from=builder /app/migrations /app/migrations

WORKDIR /app

# Expose ports
EXPOSE 8080 8081

# Environment variables (must be provided at runtime)
ENV SOHBET_JWT_SECRET=${SOHBET_JWT_SECRET}
ENV DATABASE_URL=${DATABASE_URL}

# Run server
CMD ["./sohbet"]
