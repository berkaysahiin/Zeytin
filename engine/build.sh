#!/bin/bash

set -e  

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENGINE_DIR="$SCRIPT_DIR"

echo "Building Zeytin Engine..."
echo "Engine directory: $ENGINE_DIR"

mkdir -p "$ENGINE_DIR/build"
cd "$ENGINE_DIR/build"

echo "Configuring CMake with Ninja generator..."
cmake .. -G Ninja -DENGINE_CONFIG=weditor

echo "Building the first pass for preparser..."
cmake --build . -j

echo "Running preparser to generate component code..."
cd "$ENGINE_DIR"
./preparser "$ENGINE_DIR/build"

echo "Rebuilding with generated code..."
cd "$ENGINE_DIR/build"
cmake --build . -j

if [ ! -f "$ENGINE_DIR/build/ZeytinEngine_weditor" ]; then
    echo "ERROR: Build reported success but executable not found at $ENGINE_DIR/build/ZeytinEngine_weditor" >&2
    exit 1
fi

echo "Build successful!"
exit 0
