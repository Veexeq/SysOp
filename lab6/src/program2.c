#ifdef LEVEL2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "common.h"

// --- GLOBAL FLAGS & STATS ---
volatile bool is_running = true;

int stats_frames_generated = 0;
int stats_robot_states = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    
    // Broadcast condition variables to wake up any blocked threads (buffer_pop)
    pthread_cond_broadcast(&cam_left_buf.not_empty);
    pthread_cond_broadcast(&cam_right_buf.not_empty);
    pthread_cond_broadcast(&sync_left_buf.not_empty);
    pthread_cond_broadcast(&sync_right_buf.not_empty);
}

// --- THREAD FUNCTIONS ---

void* left_camera_thread(void* arg) {
    (void)arg;
    int id = 1;
    while (is_running) {
        frame_t frame = { .frame_id = id++ };
        get_current_time(&frame.timestamp);
        
        buffer_push(&cam_left_buf, frame);
        
        pthread_mutex_lock(&stats_mutex);
        stats_frames_generated++;
        pthread_mutex_unlock(&stats_mutex);
        
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
        
        sleep_for_freq(CAMERA_FREQ_HZ);
    }
    return NULL;
}

void* sync_thread(void* arg) {
    (void)arg;
    frame_t left, right;
    while (is_running) {
        // Blocks efficiently using condition variables until a frame arrives
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
        
        // Writer limits its disk operations to 10 Hz
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
        
        pthread_mutex_lock(&stats_mutex);
        stats_robot_states++;
        pthread_mutex_unlock(&stats_mutex);
        
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

// --- MAIN FUNCTION ---

int main() {
    printf("[LEVEL 2] System starting. Press CTRL+C to stop.\n");
    signal(SIGINT, handle_sigint);
    
    buffer_init(&cam_left_buf);
    buffer_init(&cam_right_buf);
    buffer_init(&sync_left_buf);
    buffer_init(&sync_right_buf);
    
    pthread_t threads[6];
    pthread_create(&threads[0], NULL, left_camera_thread, NULL);
    pthread_create(&threads[1], NULL, right_camera_thread, NULL);
    pthread_create(&threads[2], NULL, sync_thread, NULL);
    pthread_create(&threads[3], NULL, image_writer_thread, NULL);
    pthread_create(&threads[4], NULL, robot_state_thread, NULL);
    pthread_create(&threads[5], NULL, logger_thread, NULL);
    
    // Main thread now handles System Statistics reporting
    while (is_running) {
        sleep(2); // Print stats every 2 seconds
        if (!is_running) break;
        
        pthread_mutex_lock(&stats_mutex);
        int f_count = stats_frames_generated;
        int r_count = stats_robot_states;
        pthread_mutex_unlock(&stats_mutex);
        
        printf("[STATS] Camera Frames: %d | Robot States: %d | C_Freq: 25Hz | R_Freq: 100Hz\n", 
               f_count, r_count);
    }
    
    for (int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }
    
    buffer_destroy(&cam_left_buf);
    buffer_destroy(&cam_right_buf);
    buffer_destroy(&sync_left_buf);
    buffer_destroy(&sync_right_buf);
    
    printf("[LEVEL 2] System shutdown gracefully.\n");
    return 0;
}

#endif // LEVEL2
