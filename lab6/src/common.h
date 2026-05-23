#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>

// --- SYSTEM CONSTANTS ---
#define CAMERA_FREQ_HZ 25
#define ROBOT_FREQ_HZ 100
#define LOG_FREQ_HZ 10
#define SYNC_TOLERANCE_MS 20  // Max time difference for stereo matching (20 ms)
#define BUFFER_SIZE 64        // Circular buffer size

// --- DATA STRUCTURES ---

// Single camera frame structure
typedef struct {
    int frame_id;               // Sequential frame number
    struct timespec timestamp;  // Time of capture
} frame_t;

// Robot state structure
typedef struct {
    double x;
    double y;
    double theta;               // Orientation
    struct timespec timestamp;  // State generation time
} robot_state_t;

// Circular buffer structure (used from Task 2)
typedef struct {
    frame_t buffer[BUFFER_SIZE];
    int head;                 // Write index
    int tail;                 // Read index
    int count;                // Current number of elements
    
    // Synchronization mechanisms (Task 2)
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} frame_buffer_t;

// --- UTILITY FUNCTION DECLARATIONS ---

// Calculates time difference in milliseconds between two timestamps
double time_diff_ms(struct timespec start, struct timespec end);

// Gets current time using CLOCK_MONOTONIC
void get_current_time(struct timespec *ts);

// Precise thread sleep for a given frequency (e.g., 25 Hz)
void sleep_for_freq(int freq_hz);

#endif // COMMON_H
