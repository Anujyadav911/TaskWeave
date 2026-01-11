#!/bin/bash
# TaskWeave CMake Build Script for Linux/macOS

set -e

echo "=========================================="
echo "TaskWeave - CMake Build Script"
echo "=========================================="
echo ""

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# Build
echo ""
echo "Building TaskWeave..."
cmake --build . -j$(nproc)

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "Executable location: $(pwd)/taskweave"
    echo ""
    echo "To run:"
    echo "  cd build && ./taskweave --mode=api"
    echo "  OR"
    echo "  ./build/taskweave --mode=api"
else
    echo ""
    echo "❌ Build failed. Check errors above."
    exit 1
fi
