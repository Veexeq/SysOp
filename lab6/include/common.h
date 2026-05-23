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

// --- BUFFER OPERATIONS (Task 2) ---

// Initializes the circular buffer and its synchronization mechanisms
void buffer_init(frame_buffer_t* buf);

// Pushes a frame into the buffer. Overwrites the oldest frame if full.
void buffer_push(frame_buffer_t* buf, frame_t frame);

// Pops a frame from the buffer. Blocks if empty. Returns false if interrupted.
bool buffer_pop(frame_buffer_t* buf, frame_t* frame, volatile bool* is_running);

// Cleans up mutexes and condition variables
void buffer_destroy(frame_buffer_t* buf);

#endif // COMMON_H
