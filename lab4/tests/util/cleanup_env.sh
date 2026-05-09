#!/bin/bash

echo "[Cleanup] Restoring system to initial state..."

# 1. Zabijanie zawieszonych procesów w tle (jeśli istnieją)
# Używamy '|| true', aby skrypt nie przerwał działania, gdy nie znajdzie procesów do ubicia
pkill -f "server.out" 2>/dev/null || true
pkill -f "client.out" 2>/dev/null || true

# 2. Usuwanie kolejek POSIX z wirtualnego systemu plików
if [ -d "/dev/mqueue" ]; then
    rm -f /dev/mqueue/chat_server_q 2>/dev/null || true
    rm -f /dev/mqueue/chat_client_* 2>/dev/null || true
fi

# 3. Usuwanie nazwanych potoków (FIFOs)
rm -f /tmp/c1_in /tmp/c2_in 2>/dev/null || true

echo "[Cleanup] Environment reset completed."
