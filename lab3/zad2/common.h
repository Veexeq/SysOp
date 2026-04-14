#pragma once

// Ścieżki do naszych potoków nazwanych (plików FIFO) na dysku
#define FIFO_REQ "/tmp/fifo_request"   // Kierunek: Program 1 -> Program 2
#define FIFO_RES "/tmp/fifo_response"  // Kierunek: Program 2 -> Program 1

// Struktura reprezentująca dane, które będziemy przesyłać
typedef struct {
    double start;
    double end;
    double dx;
} Interval;
