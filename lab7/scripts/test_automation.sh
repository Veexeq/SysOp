#!/bin/bash

# ==============================================================================
# BULLETPROOF TMUX Automation Script for Socket Server/Client Testing
# ==============================================================================

SESSION_NAME="socket_test_session"

# Calculate the absolute path of the project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change the working directory of this script execution to the project root
cd "$PROJECT_ROOT" || exit 1

# Ensure tmux is installed in WSL2
if ! command -v tmux &> /dev/null; then
    echo "Error: tmux is not installed. Please run 'sudo apt install tmux' first."
    exit 1
fi

# Ensure the binaries exist before running the automation
if [ ! -f "./bin/server" ] || [ ! -f "./bin/client" ]; then
    echo "Error: Binaries not found. Please build the project using 'make' first."
    exit 1
fi

echo "Starting automated test architecture in: $PROJECT_ROOT"

# 1. Start a new detached tmux session
tmux new-session -d -s "$SESSION_NAME" -n "Main"

# 2. FORCE Pane 0 (Server) to change directory to project root via explicit send-keys
# This overrides any global shell settings or .bashrc profiles
tmux send-keys -t "$SESSION_NAME:Main.0" "cd \"$PROJECT_ROOT\"" C-m
tmux send-keys -t "$SESSION_NAME:Main.0" "./bin/server" C-m

# 3. Give the server a moment to bind to port 9000
sleep 1.5

# 4. Split the current window horizontally to create Pane 1 (Client)
tmux split-window -h -t "$SESSION_NAME:Main"

# 5. Run the client automation in the BACKGROUND.
(
    sleep 2.0
    
    # FORCE Pane 1 (Client) to change directory to project root before sending commands
    tmux send-keys -t "$SESSION_NAME:Main.1" "cd \"$PROJECT_ROOT\"" C-m
    sleep 0.5

    echo "-> Sending Request 1 (Value: 10) via client..."
    tmux send-keys -t "$SESSION_NAME:Main.1" "./bin/client 127.0.0.1 9000 10" C-m
    sleep 2.0

    echo "-> Sending Request 2 (Value: 50) via client..."
    tmux send-keys -t "$SESSION_NAME:Main.1" "./bin/client 127.0.0.1 9000 50" C-m
    sleep 2.0

    echo "-> Sending Request 3 (Value: 15) via client..."
    tmux send-keys -t "$SESSION_NAME:Main.1" "./bin/client 127.0.0.1 9000 15" C-m
    sleep 3.0
    
    # Trigger graceful shutdown of the server (sending SIGINT via Ctrl+C)
    echo "-> Triggering graceful server shutdown (sending SIGINT)..."
    tmux send-keys -t "$SESSION_NAME:Main.0" C-c
) &

# 6. Immediately attach to the tmux session so the user can watch the live show
echo "Attaching to tmux session... Watch the automation live!"
tmux attach-session -t "$SESSION_NAME"