# Multi-stage build for TaskWeave
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application using CMake
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc) && \
    cp taskweave .. && \
    cd .. && rm -rf build

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary and config files
COPY --from=builder /app/taskweave .
COPY --from=builder /app/src/config.ini .
COPY --from=builder /app/web ./web
COPY tasks.json .


# Create directory for database
RUN mkdir -p /app/data

# Expose API port
EXPOSE 8080

# Set environment variables
ENV TASKWEAVE_DB_PATH=/app/data/taskweave.db
ENV TASKWEAVE_LOG_PATH=/app/data/taskweave.log

# Run the application
CMD ["./taskweave", "--mode=api", "--api-port=8080"]
