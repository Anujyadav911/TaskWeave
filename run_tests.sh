#!/bin/bash
# TaskWeave Test Runner Script for Linux/macOS

set -e

echo "=========================================="
echo "TaskWeave - Running Tests"
echo "=========================================="
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Building project first..."
    ./build_cmake.sh
fi

cd build

# Check if CMake needs to be reconfigured for tests
NEEDS_RECONFIGURE=false
if [ ! -f "CMakeCache.txt" ]; then
    NEEDS_RECONFIGURE=true
elif ! grep -q "BUILD_TESTING:BOOL=ON" CMakeCache.txt 2>/dev/null; then
    NEEDS_RECONFIGURE=true
fi

if [ "$NEEDS_RECONFIGURE" = true ]; then
    echo "Reconfiguring CMake with tests enabled..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    
    if [ $? -ne 0 ]; then
        echo ""
        echo "❌ CMake reconfiguration failed."
        cd ..
        exit 1
    fi
fi

# Check if test executable exists
if [ ! -f "taskweave_tests" ]; then
    echo "Building tests..."
    cmake --build . --target taskweave_tests
    
    if [ $? -ne 0 ]; then
        echo ""
        echo "❌ Test build failed."
        cd ..
        exit 1
    fi
fi

echo "Running tests..."
echo ""

# Run tests using CTest
ctest --output-on-failure --verbose

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ All tests passed!"
else
    echo ""
    echo "❌ Some tests failed. See output above."
    cd ..
    exit 1
fi

cd ..
