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

// --- BUFFER IMPLEMENTATION ---

void buffer_init(frame_buffer_t* buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
    pthread_cond_init(&buf->not_full, NULL);
}

void buffer_push(frame_buffer_t* buf, frame_t frame) {
    pthread_mutex_lock(&buf->mutex);
    
    // Real-Time System approach: If buffer is full, drop the oldest frame
    // to prevent blocking the high-frequency producer (camera)
    if (buf->count == BUFFER_SIZE) {
        buf->tail = (buf->tail + 1) % BUFFER_SIZE;
        buf->count--;
    }
    
    buf->buffer[buf->head] = frame;
    buf->head = (buf->head + 1) % BUFFER_SIZE;
    buf->count++;
    
    // Wake up any thread waiting for data (e.g., synchronizer or writer)
    pthread_cond_signal(&buf->not_empty);
    
    pthread_mutex_unlock(&buf->mutex);
}

bool buffer_pop(frame_buffer_t* buf, frame_t* frame, volatile bool* is_running) {
    pthread_mutex_lock(&buf->mutex);
    
    // Wait until there is data OR the system is shutting down
    while (buf->count == 0 && *is_running) {
        pthread_cond_wait(&buf->not_empty, &buf->mutex);
    }
    
    // If we woke up because of shutdown and buffer is empty, abort safely
    if (!(*is_running) && buf->count == 0) {
        pthread_mutex_unlock(&buf->mutex);
        return false;
    }
    
    *frame = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % BUFFER_SIZE;
    buf->count--;
    
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
    return true;
}

void buffer_destroy(frame_buffer_t* buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->not_empty);
    pthread_cond_destroy(&buf->not_full);
}
