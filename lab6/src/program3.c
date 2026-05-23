#ifdef LEVEL3

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <sched.h>
#include "common.h"

// --- GLOBAL FLAGS ---
volatile bool is_running = true;

// --- ATOMIC COUNTERS (Lock-free synchronization) ---
// Total counters for final report
atomic_int total_left_frames = 0;
atomic_int total_right_frames = 0;
atomic_int total_robot_states = 0;

// Recent counters for the watchdog (reset every second)
atomic_int recent_left_frames = 0;
atomic_int recent_right_frames = 0;
atomic_int recent_robot_states = 0;

// --- SHARED BUFFERS & STATE ---
frame_buffer_t cam_left_buf;
frame_buffer_t cam_right_buf;
frame_buffer_t sync_left_buf;
frame_buffer_t sync_right_buf;

robot_state_t current_robot_state;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

// --- SIGNAL HANDLER ---
void handle_sigint(int sig) {
    (void)sig;
    printf("\n[SIGINT] CTRL+C detected. Initiating graceful shutdown...\n");
    is_running = false;
    
    pthread_cond_broadcast(&cam_left_buf.not_empty);
    pthread_cond_broadcast(&cam_right_buf.not_empty);
    pthread_cond_broadcast(&sync_left_buf.not_empty);
    pthread_cond_broadcast(&sync_right_buf.not_empty);
}

// --- UTILITY: SET THREAD PRIORITY ---
void set_thread_priority(pthread_t thread, int policy, int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    
    // SCHED_FIFO requires elevated privileges (sudo/root) in Linux.
    // If it fails, it will gracefully fall back to default scheduling and print a warning.
    if (pthread_setschedparam(thread, policy, &param) != 0) {
        // We only print warning once to avoid console spam
        static atomic_bool warned = false;
        bool expected = false;
        if (atomic_compare_exchange_strong(&warned, &expected, true)) {
            perror("[WARNING] Failed to set Real-Time priority (try running with sudo)");
        }
    }
}

// --- THREAD FUNCTIONS ---

void* left_camera_thread(void* arg) {
    (void)arg;
    int id = 1;
    while (is_running) {
        frame_t frame = { .frame_id = id++ };
        get_current_time(&frame.timestamp);
        
        buffer_push(&cam_left_buf, frame);
        
        // Atomic increments (no mutex required)
        atomic_fetch_add(&total_left_frames, 1);
        atomic_fetch_add(&recent_left_frames, 1);
        
        sleep_for_freq(CAMERA_FREQ_HZ);
    }
    return NULL;
}

void* right_camera_thread(void* arg) {
    (void)arg;
    int id = 1;
    while (is_running) {
        frame_t frame = { .frame_id = id++ };
        get_current_time(&frame.timestamp);
        
        buffer_push(&cam_right_buf, frame);
        
        atomic_fetch_add(&total_right_frames, 1);
        atomic_fetch_add(&recent_right_frames, 1);
        
        sleep_for_freq(CAMERA_FREQ_HZ);
    }
    return NULL;
}

void* sync_thread(void* arg) {
    (void)arg;
    frame_t left, right;
    while (is_running) {
        if (!buffer_pop(&cam_left_buf, &left, &is_running)) break;
        if (!buffer_pop(&cam_right_buf, &right, &is_running)) break;
        
        double diff = time_diff_ms(left.timestamp, right.timestamp);
        if (diff < 0) diff = -diff;
        
        if (diff < SYNC_TOLERANCE_MS) {
            buffer_push(&sync_left_buf, left);
            buffer_push(&sync_right_buf, right);
        }
    }
    return NULL;
}

void* image_writer_thread(void* arg) {
    (void)arg;
    frame_t left, right;
    while (is_running) {
        if (!buffer_pop(&sync_left_buf, &left, &is_running)) break;
        if (!buffer_pop(&sync_right_buf, &right, &is_running)) break;
        
        char filename_left[64], filename_right[64];
        snprintf(filename_left, sizeof(filename_left), "output/left_%04d.jpg", left.frame_id);
        snprintf(filename_right, sizeof(filename_right), "output/right_%04d.jpg", right.frame_id);
        
        FILE* fl = fopen(filename_left, "w");
        if (fl) { fprintf(fl, "Sync L: %d\n", left.frame_id); fclose(fl); }
        FILE* fr = fopen(filename_right, "w");
        if (fr) { fprintf(fr, "Sync R: %d\n", right.frame_id); fclose(fr); }
        
        sleep_for_freq(LOG_FREQ_HZ);
    }
    return NULL;
}

void* robot_state_thread(void* arg) {
    (void)arg;
    double x = 0.0, y = 0.0, theta = 0.0;
    while (is_running) {
        x += 0.01; y += 0.01; theta += 0.005;
        
        pthread_mutex_lock(&state_mutex);
        current_robot_state.x = x;
        current_robot_state.y = y;
        current_robot_state.theta = theta;
        get_current_time(&current_robot_state.timestamp);
        pthread_mutex_unlock(&state_mutex);
        
        atomic_fetch_add(&total_robot_states, 1);
        atomic_fetch_add(&recent_robot_states, 1);
        
        sleep_for_freq(ROBOT_FREQ_HZ);
    }
    return NULL;
}

void* logger_thread(void* arg) {
    (void)arg;
    FILE* log_file = fopen("output/robot_state.txt", "w");
    if (!log_file) return NULL;
    
    while (is_running) {
        pthread_mutex_lock(&state_mutex);
        robot_state_t state = current_robot_state;
        pthread_mutex_unlock(&state_mutex);
        
        fprintf(log_file, "X: %.2f, Y: %.2f, Theta: %.2f\n", state.x, state.y, state.theta);
        fflush(log_file);
        
        sleep_for_freq(LOG_FREQ_HZ);
    }
    fclose(log_file);
    return NULL;
}

void* watchdog_thread(void* arg) {
    (void)arg;
    while (is_running) {
        sleep(1); // Watchdog checks system pulse every 1 second
        if (!is_running) break;
        
        // Read and reset counters using atomic exchange
        int l_frames = atomic_exchange(&recent_left_frames, 0);
        int r_frames = atomic_exchange(&recent_right_frames, 0);
        int r_states = atomic_exchange(&recent_robot_states, 0);
        
        // Check for missed deadlines (allowing a small margin for standard OS delays)
        if (l_frames < CAMERA_FREQ_HZ - 2) {
            printf("[WATCHDOG] LEFT CAMERA SLOW: %d Hz (Expected: %d Hz)\n", l_frames, CAMERA_FREQ_HZ);
        }
        if (r_frames < CAMERA_FREQ_HZ - 2) {
            printf("[WATCHDOG] RIGHT CAMERA SLOW: %d Hz (Expected: %d Hz)\n", r_frames, CAMERA_FREQ_HZ);
        }
        if (r_states < ROBOT_FREQ_HZ - 5) {
            printf("[WATCHDOG] ROBOT STATE SLOW: %d Hz (Expected: %d Hz)\n", r_states, ROBOT_FREQ_HZ);
        }
    }
    return NULL;
}

// --- MAIN FUNCTION ---

int main() {
    printf("[LEVEL 3] RTOS System starting. Press CTRL+C to stop.\n");
    signal(SIGINT, handle_sigint);
    
    buffer_init(&cam_left_buf);
    buffer_init(&cam_right_buf);
    buffer_init(&sync_left_buf);
    buffer_init(&sync_right_buf);
    
    pthread_t threads[7];
    
    // Create threads
    pthread_create(&threads[0], NULL, left_camera_thread, NULL);
    pthread_create(&threads[1], NULL, right_camera_thread, NULL);
    pthread_create(&threads[2], NULL, sync_thread, NULL);
    pthread_create(&threads[3], NULL, image_writer_thread, NULL);
    pthread_create(&threads[4], NULL, robot_state_thread, NULL);
    pthread_create(&threads[5], NULL, logger_thread, NULL);
    pthread_create(&threads[6], NULL, watchdog_thread, NULL); // NEW: Watchdog
    
    // Assign Real-Time priorities (SCHED_FIFO)
    // Higher number = higher priority
    set_thread_priority(threads[4], SCHED_FIFO, 90); // Robot State - Highest priority
    set_thread_priority(threads[6], SCHED_FIFO, 85); // Watchdog - Critical monitoring
    set_thread_priority(threads[0], SCHED_FIFO, 80); // Camera Left
    set_thread_priority(threads[1], SCHED_FIFO, 80); // Camera Right
    
    // Main thread acts as idle sleep (Watchdog handles monitoring now)
    while (is_running) {
        sleep(1);
    }
    
    // Join threads
    for (int i = 0; i < 7; i++) {
        pthread_join(threads[i], NULL);
    }
    
    buffer_destroy(&cam_left_buf);
    buffer_destroy(&cam_right_buf);
    buffer_destroy(&sync_left_buf);
    buffer_destroy(&sync_right_buf);
    
    // Generate Final Report
    printf("[LEVEL 3] Generating final report...\n");
    FILE* report = fopen("output/report.txt", "w");
    if (report) {
        fprintf(report, "===================================\n");
        fprintf(report, "    SYSTEM PERFORMANCE REPORT      \n");
        fprintf(report, "===================================\n");
        fprintf(report, "Total Left Camera Frames  : %d\n", atomic_load(&total_left_frames));
        fprintf(report, "Total Right Camera Frames : %d\n", atomic_load(&total_right_frames));
        fprintf(report, "Total Robot States Logged : %d\n", atomic_load(&total_robot_states));
        fprintf(report, "Expected camera frequency : %d Hz\n", CAMERA_FREQ_HZ);
        fprintf(report, "Expected robot frequency  : %d Hz\n", ROBOT_FREQ_HZ);
        fprintf(report, "===================================\n");
        fclose(report);
    }
    
    printf("[LEVEL 3] System shutdown gracefully.\n");
    return 0;
}

#endif // LEVEL3
