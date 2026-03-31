#!/bin/bash

TARGETS=("static" "shared" "dynamic")
MODES=("default" "mask" "ignore" "handle")

echo "========================================"
echo " Uruchamiam zautomatyzowane testy (CI)  "
echo "========================================"

# Zewnętrzna pętla: iterujemy po rodzajach bibliotek
for target in "${TARGETS[@]}"; do
    echo -e "\n---> Budowanie środowiska: $target"
    
    # Ciche budowanie projektu
    make clean > /dev/null 2>&1
    make $target > /dev/null 2>&1
    
    # Wewnętrzna pętla: testujemy każdy tryb
    for mode in "${MODES[@]}"; do
        # Opcja -n sprawia, że echo nie przechodzi do nowej linii
        echo -n "Test ./main.out $mode ... "
        
        # 1. PRZECHWYCENIE WYJŚCIA
        # Uruchamiamy program, a cały jego tekst zapisujemy do zmiennej OUTPUT
        # (Trwa to 20 sekund dla pętli, które dochodzą do końca)
        OUTPUT=$(./main.out $mode 2>&1)
        
        PASSED=0 # Zmienna flagowa (0 = FAIL, 1 = PASS)
        
        # 2. LOGIKA ASERCJI (Sprawdzanie warunków)
        if [[ "$mode" == "ignore" || "$mode" == "handle" ]]; then
            # Oczekujemy, że program wypisał informację o końcu pętli
            if echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
            
        elif [[ "$mode" == "default" ]]; then
            # Oczekujemy strzału, ale braku końca pętli
            if echo "$OUTPUT" | grep -q "Wysyłam sygnał USR1" && ! echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
            
        elif [[ "$mode" == "mask" ]]; then
            # Oczekujemy zdjęcia maski, ale braku końca pętli
            if echo "$OUTPUT" | grep -q "Odblokowuję USR1" && ! echo "$OUTPUT" | grep -q "Pętla została wykonana w całości"; then
                PASSED=1
            fi
        fi
        
        # 3. WYPISANIE WYNIKU Z KOLORAMI
        if [ $PASSED -eq 1 ]; then
            # \e[32m to kod koloru zielonego, \e[0m resetuje kolor
            echo -e "\e[32m[PASS]\e[0m"
        else
            # \e[31m to kod koloru czerwonego
            echo -e "\e[31m[FAIL]\e[0m"
            echo "--- Zrzut ekranu z błędem ---"
            echo "$OUTPUT"
            echo "-----------------------------"
        fi
        
    done
done

echo -e "\n========================================"
echo " Wszystkie testy zostały zakończone!    "
echo "========================================"
