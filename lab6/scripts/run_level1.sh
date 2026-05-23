#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "========================================"
echo "    Building and Running LEVEL 1        "
echo "========================================"

make clean > /dev/null 2>&1
make level1

echo ""
echo "[INFO] Starting simulation (takes 20 seconds)..."
echo "----------------------------------------"

./bin/level1.out

echo "----------------------------------------"
echo "[INFO] Simulation finished. Analyzing outputs..."
echo ""

# Check outputs in the new directory
if [ -f "output/robot_state.txt" ]; then
    echo "=> Robot State Log (first 5 entries):"
    head -n 5 output/robot_state.txt
else
    echo "=> [ERROR] output/robot_state.txt was not generated!"
fi

echo ""

IMAGE_COUNT=$(ls -1 output/*.jpg 2>/dev/null | wc -l)
if [ "$IMAGE_COUNT" -gt 0 ]; then
    echo "=> Total simulated images generated: $IMAGE_COUNT"
    echo "=> Sample generated image files:"
    ls -1 output/left_*.jpg output/right_*.jpg 2>/dev/null | head -n 6
else
    echo "=> [ERROR] No .jpg files were generated in output/!"
fi

echo "========================================"
echo "    Script Execution Completed          "
echo "========================================"
