#!/bin/bash

# Run a cleanup script pre-demo
cd "$(dirname "$0")"
./util/cleanup_env.sh

cd ../
set -e
make all

echo "====================================================="
echo "  Starting automated IPC Chat demonstration"
echo "====================================================="

# Create named pipes (FIFOs) to serve as virtual keyboards
mkfifo /tmp/c1_in
mkfifo /tmp/c2_in

# 1. Start the server in the background and save its PID
./bin/server.out &
SERVER_PID=$!
echo "[System] Server initialized (PID: $SERVER_PID)"
sleep 1 # Allow the server a moment to create the main queue

# 2. Start clients in the background, redirecting their input (<) from FIFOs
./bin/client.out < /tmp/c1_in &
C1_PID=$!
echo "[System] Client 1 initialized (PID: $C1_PID)"

./bin/client.out < /tmp/c2_in &
C2_PID=$!
echo "[System] Client 2 initialized (PID: $C2_PID)"

sleep 1 # Wait for clients to send MSG_INIT and receive their IDs

# Open the FIFOs for writing by assigning them file descriptors 3 and 4
exec 3> /tmp/c1_in
exec 4> /tmp/c2_in

echo -e "\n[System] Initialization complete. Commencing message exchange simulation...\n"
sleep 1

# 3. Simulate text input (sending text to descriptors 3 and 4)
echo "Hello team. This is Client 1 verifying the connection. Can you confirm receipt?" >&3
sleep 2

echo "Client 2 confirming receipt. The connection is stable and latency is minimal." >&4
sleep 2

echo "Excellent. It appears the POSIX message queues are routing data correctly." >&3
sleep 2

echo "Agreed. The IPC implementation is functioning as expected. The system is ready." >&4
sleep 2

# 4. Automated cleanup
echo -e "\n[System] Demonstration concluded. Terminating processes and cleaning up resources..."

# Close the file descriptors
exec 3>&-
exec 4>&-

# Send SIGTERM to the processes to trigger their cleanup handlers
kill -TERM $C1_PID $C2_PID $SERVER_PID 2>/dev/null

# Wait for processes to exit to avoid messing up the terminal prompt
wait $SERVER_PID 2>/dev/null || true

# Remove the virtual keyboards
rm /tmp/c1_in /tmp/c2_in

echo "[System] Cleanup successful. Exiting."
