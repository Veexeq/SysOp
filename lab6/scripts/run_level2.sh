#!/bin/bash

set -e
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "========================================"
echo "    Building and Running LEVEL 2        "
echo "========================================"

make clean > /dev/null 2>&1
make level2

echo ""
echo "[INFO] System runs infinitely. We will simulate CTRL+C after 8 seconds..."
echo "----------------------------------------"

# Run program in the background
./bin/level2.out &
PID=$!

# Wait for 8 seconds, letting the program print some stats
sleep 8

# Send SIGINT (CTRL+C) to the process
echo ""
echo "[BASH] Sending SIGINT to process $PID..."
kill -INT $PID

# Wait for the background process to finish gracefully
wait $PID

echo "----------------------------------------"
echo "[INFO] System shutdown verified. Checking outputs..."
echo ""

IMAGE_COUNT=$(ls -1 output/*.jpg 2>/dev/null | wc -l)
if [ "$IMAGE_COUNT" -gt 0 ]; then
    echo "=> Total simulated images generated: $IMAGE_COUNT"
else
    echo "=> [ERROR] No .jpg files found!"
fi

echo "========================================"
echo "    Level 2 Completed!                  "
echo "========================================"
