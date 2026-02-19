#!/bin/bash

set -e

echo "Building project..."
cmake -B cmake-build-debug -S .
cmake --build cmake-build-debug

echo ""
echo "Running tests..."
echo "================="
cd cmake-build-debug
ctest --output-on-failure

echo ""
echo "All tests completed successfully!"

