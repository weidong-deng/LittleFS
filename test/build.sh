#!/bin/bash
# LittleFS Test Build Script

# Enter script directory
cd "$(dirname "$0")"

# Check tools
echo "Checking build environment..."
if ! command -v gcc &>/dev/null; then
    echo "ERROR: GCC not found"
    exit 1
fi

if ! command -v cmake &>/dev/null; then
    echo "ERROR: CMake not found"
    exit 1
fi

echo "GCC and CMake OK"

# Remove old build directory
if [ -d build ]; then
    echo "Removing old build directory..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir build
if [ $? -ne 0 ]; then
    echo "ERROR: Cannot create build directory"
    exit 1
fi

# Run CMake
cd build
echo "Running CMake configure..."
cmake ..
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configure failed"
    cd ..
    exit 1
fi

# Build
echo "Building..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    cd ..
    exit 1
fi

echo ""
echo "========================================"
echo "Build SUCCESS"
echo "========================================"
echo ""

# Run tests
read -p "Run tests? (Y/N): " run_tests
if [ "$run_tests" = "Y" ] || [ "$run_tests" = "y" ]; then
    echo "Running tests..."
    ctest --output-on-failure | grep -v "Start "

    echo ""
    echo "Generating coverage report..."
    mkdir -p coverage
    gcovr --root ../.. --exclude ".*Unity.*" --html --html-details -o coverage/coverage_report.html .
    if [ $? -ne 0 ]; then
        echo "Coverage tool not available, skipping coverage report"
    else
        echo "Coverage report generated: coverage/coverage_report.html"
    fi
fi

cd ..
