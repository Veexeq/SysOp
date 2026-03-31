#!/bin/bash

TARGETS=("static" "shared" "dynamic")
MODES=("default" "mask" "ignore" "handle")
LOG_DIR="logs"

echo "========================================"
echo " Uruchamiam zautomatyzowane testy (CI)  "
echo " Czas trwania: ok. 60 sekund            "
echo "========================================"

# Tworzymy czysty folder na logi (ukrywamy błędy, jeśli już istnieje)
mkdir -p "$LOG_DIR"

# Zewnętrzna pętla: iterujemy po rodzajach bibliotek (muszą być po kolei)
for target in "${TARGETS[@]}"; do
    echo -e "\n---> Budowanie środowiska: $target"
    
    make clean > /dev/null 2>&1
    make $target > /dev/null 2>&1
    
    echo "Uruchamiam 4 testy równolegle w tle..."
    
    # =======================================================
    # KROK 1: Uruchomienie równoległe (zapis do folderu logs/)
    # =======================================================
    for mode in "${MODES[@]}"; do
        stdbuf -o0 ./main.out $mode > "$LOG_DIR/test_${target}_${mode}.log" 2>&1 &
    done
    
    # Czekamy na zakończenie wszystkich 4 procesów w tle
    wait 
    
    # =======================================================
    # KROK 2: Sekwencyjna weryfikacja wyników
    # =======================================================
    for mode in "${MODES[@]}"; do
        echo -n "Test ./main.out $mode ... "
        
        # Wczytujemy logi z nowego folderu
        OUTPUT=$(cat "$LOG_DIR/test_${target}_${mode}.log")
        PASSED=0
        
        if [[ "$mode" == "ignore" || "$mode" == "handle" ]]; then
            if echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
            
        elif [[ "$mode" == "default" ]]; then
            if echo "$OUTPUT" | grep -q "Wysyłam sygnał USR1" && ! echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
            
        elif [[ "$mode" == "mask" ]]; then
            if echo "$OUTPUT" | grep -q "Odblokowuję USR1" && ! echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
        fi
        
        # Kolorowanie wyników
        if [ $PASSED -eq 1 ]; then
            echo -e "\e[32m[PASS]\e[0m"
        else
            echo -e "\e[31m[FAIL]\e[0m"
            echo "--- Zrzut ekranu z pliku $LOG_DIR/test_${target}_${mode}.log ---"
            echo "$OUTPUT"
            echo "---------------------------------------------------------"
        fi
    done
done

# =======================================================
# KROK 3: Sprzątanie całego folderu z logami
# =======================================================
# rm -rf "$LOG_DIR"

echo -e "\n========================================"
echo " Wszystkie testy zostały zakończone!    "
echo "========================================"
