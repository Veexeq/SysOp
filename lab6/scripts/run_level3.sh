#!/bin/bash

set -e
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "========================================"
echo "    Building and Running LEVEL 3        "
echo "========================================"

make clean > /dev/null 2>&1
make level3

echo ""
echo "[INFO] Running RTOS Watchdog simulation for 10 seconds..."
echo "       (Note: You may see a permission warning for Real-Time priority if not running as sudo. This is perfectly normal on Linux.)"
echo "----------------------------------------"

./bin/level3.out &
PID=$!

sleep 10

echo ""
echo "[BASH] Sending SIGINT to process $PID..."
kill -INT $PID
wait $PID

echo "----------------------------------------"
echo "[INFO] System shutdown verified. Checking final report..."
echo ""

if [ -f "output/report.txt" ]; then
    cat output/report.txt
else
    echo "=> [ERROR] output/report.txt was not generated!"
fi

echo "========================================"
echo "    Level 3 Completed!                  "
echo "========================================"
