#!/bin/bash

set -e  

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENGINE_DIR="$SCRIPT_DIR"

"$ENGINE_DIR/build.sh"

echo "Launching engine..."

cd "$ENGINE_DIR/build"
nohup ./ZeytinEngine_weditor > /dev/null 2>&1 &
ENGINE_PID=$!

echo "Engine started with PID: $ENGINE_PID"
exit 0
