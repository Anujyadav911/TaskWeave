#!/bin/bash
# TaskWeave Build Script for Linux/macOS

echo "Building TaskWeave..."

g++ -std=c++17 \
  -Isrc \
  -Icore \
  -Iapi \
  -Iutils \
  src/main.cpp \
  src/core/Task.cpp \
  core/TaskLoader.cpp \
  core/TaskRegistry.cpp \
  src/executor/ThreadPool.cpp \
  src/scheduler/PriorityScheduler.cpp \
  src/scheduler/RoundRobinScheduler.cpp \
  api/ApiServer.cpp \
  utils/Config.cpp \
  utils/Metrics.cpp \
  utils/Database.cpp \
  -o taskweave \
  -pthread \
  -lsqlite3

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful! Run with: ./taskweave --mode=api"
else
    echo ""
    echo "❌ Build failed. Check errors above."
fi
