#include "common.h"
#include <time.h>
#include <unistd.h>
#include <stddef.h>

// Calculates time difference in milliseconds between two timestamps
double time_diff_ms(struct timespec start, struct timespec end) {
    double start_ms = (double)start.tv_sec * 1000.0 + (double)start.tv_nsec / 1000000.0;
    double end_ms = (double)end.tv_sec * 1000.0 + (double)end.tv_nsec / 1000000.0;
    
    return end_ms - start_ms;
}

// Gets current time using CLOCK_MONOTONIC to ensure it is not affected by system time changes
void get_current_time(struct timespec *ts) {
    if (ts != NULL) {
        clock_gettime(CLOCK_MONOTONIC, ts);
    }
}

// Suspends execution of the calling thread to match the desired frequency
void sleep_for_freq(int freq_hz) {
    if (freq_hz <= 0) {
        return;
    }

    // Calculate period duration in nanoseconds
    // 1 Hz = 1 second = 1,000,000,000 nanoseconds
    long period_ns = 1000000000L / freq_hz;
    
    struct timespec req;
    req.tv_sec = period_ns / 1000000000L;
    req.tv_nsec = period_ns % 1000000000L;

    // Sleep for the specified duration using monotonic clock
    // We pass 0 as the flags argument for relative sleep, and NULL for the remaining time
    clock_nanosleep(CLOCK_MONOTONIC, 0, &req, NULL);
}
