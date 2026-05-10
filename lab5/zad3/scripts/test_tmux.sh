#!/bin/bash

# Pobieramy absolutną, pełną ścieżkę do folderu, w którym leży skrypt
PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_DIR"

# 1. Kompilacja i twardy reset środowiska
make all
./bin/cleaner.out 2>/dev/null

SESSION="os_lab"

# 2. Zabijamy poprzednią sesję
tmux kill-session -t $SESSION 2>/dev/null

# 3. Tworzymy nową ukrytą sesję
tmux new-session -d -s $SESSION

# 4. Podział ekranu
tmux split-window -h
tmux select-pane -t 0
tmux split-window -v
tmux select-pane -t 2
tmux split-window -v

# 5. Wysyłanie komend (KULOODPORNE PODEJŚCIE)
# Wymuszamy wejście do katalogu (cd), czyścimy ekran (clear) i odpalamy kod
tmux send-keys -t 0 "cd '$PROJECT_DIR' && clear && ./bin/producer.out" C-m
tmux send-keys -t 1 "cd '$PROJECT_DIR' && clear && sleep 1 && ./bin/manager.out" C-m
tmux send-keys -t 2 "cd '$PROJECT_DIR' && clear && ./bin/consumer.out" C-m
tmux send-keys -t 3 "cd '$PROJECT_DIR' && clear && ./bin/consumer.out" C-m

# 6. Podpięcie się do gotowej, działającej sesji
tmux attach-session -t $SESSION
