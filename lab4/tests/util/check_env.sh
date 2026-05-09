#!/bin/bash

echo "==================================================="
echo "       RAPORT STANU ŚRODOWISKA IPC (POSIX)         "
echo "==================================================="

echo -e "\n[1] Kolejki POSIX (/dev/mqueue):"
# Sprawdzamy zawartość katalogu. Jeśli jest pusty lub nie istnieje, wypisze stosowny komunikat.
if [ "$(ls -A /dev/mqueue 2>/dev/null)" ]; then
    ls -l /dev/mqueue
else
    echo "  -> Brak aktywnych kolejek (katalog jest pusty)."
fi

echo -e "\n[2] Nazwane potoki (FIFO) w katalogu /tmp:"
# Wypisze pliki potoków, a w przypadku błędu (np. braku plików) przekieruje go do /dev/null i wyświetli nasz tekst.
ls -l /tmp/c1_in /tmp/c2_in 2>/dev/null || echo "  -> Brak aktywnych potoków (c1_in / c2_in)."

echo -e "\n[3] Działające procesy (server.out / client.out):"
# Wyszukujemy procesy po nazwie za pomocą pgrep, flagi -l (wyświetl nazwę) i -f (pełna ścieżka).
PROCESSES=$(pgrep -lf "server\.out|client\.out" || true)

if [ -n "$PROCESSES" ]; then
    echo "$PROCESSES"
else
    echo "  -> Brak działających procesów serwera lub klienta."
fi

echo -e "\n==================================================="
