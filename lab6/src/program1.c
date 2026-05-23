#ifdef LEVEL1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "common.h"

// --- GLOBAL FLAGS ---
volatile bool is_running = true;

// --- SHARED DATA & SYNCHRONIZATION ---

// 1. Cameras <-> Synchronizer
frame_t left_camera_frame;
frame_t right_camera_frame;
pthread_mutex_t camera_mutex;
sem_t left_cam_sem;
sem_t right_cam_sem;

// 2. Synchronizer <-> Image Writer
frame_t sync_left_frame;
frame_t sync_right_frame;
bool has_new_sync_pair = false;
pthread_mutex_t sync_mutex;

// 3. Robot State <-> Logger
robot_state_t current_robot_state;
pthread_mutex_t state_mutex;

// --- THREAD FUNCTIONS ---

void* left_camera_thread(void* arg) {
    (void)arg; // Unused parameter
    int id = 1;
    
    while (is_running) {
        pthread_mutex_lock(&camera_mutex);
        left_camera_frame.frame_id = id++;
        get_current_time(&left_camera_frame.timestamp);
        pthread_mutex_unlock(&camera_mutex);
        
        // Notify synchronizer that a new left frame is ready
        sem_post(&left_cam_sem);
        
        sleep_for_freq(CAMERA_FREQ_HZ);
    }
    return NULL;
}

void* right_camera_thread(void* arg) {
    (void)arg;
    int id = 1;
    
    while (is_running) {
        pthread_mutex_lock(&camera_mutex);
        right_camera_frame.frame_id = id++;
        get_current_time(&right_camera_frame.timestamp);
        pthread_mutex_unlock(&camera_mutex);
        
        // Notify synchronizer that a new right frame is ready
        sem_post(&right_cam_sem);
        
        sleep_for_freq(CAMERA_FREQ_HZ);
    }
    return NULL;
}

void* sync_thread(void* arg) {
    (void)arg;
    
    while (is_running) {
        // Wait for both cameras to produce a frame
        sem_wait(&left_cam_sem);
        sem_wait(&right_cam_sem);
        
        if (!is_running) break; // Exit cleanly if program is stopping

        pthread_mutex_lock(&camera_mutex);
        frame_t left = left_camera_frame;
        frame_t right = right_camera_frame;
        pthread_mutex_unlock(&camera_mutex);
        
        // Calculate absolute time difference
        double diff = time_diff_ms(left.timestamp, right.timestamp);
        if (diff < 0) diff = -diff;
        
        // Create a stereo pair if timestamps are close enough
        if (diff < SYNC_TOLERANCE_MS) {
            pthread_mutex_lock(&sync_mutex);
            sync_left_frame = left;
            sync_right_frame = right;
            has_new_sync_pair = true;
            pthread_mutex_unlock(&sync_mutex);
        }
    }
    return NULL;
}

void* image_writer_thread(void* arg) {
    (void)arg;
    
    while (is_running) {
        pthread_mutex_lock(&sync_mutex);
        if (has_new_sync_pair) {
            frame_t left = sync_left_frame;
            frame_t right = sync_right_frame;
            has_new_sync_pair = false;
            pthread_mutex_unlock(&sync_mutex);
            
            // Simulate saving .jpg files in the output directory
            char filename_left[64];
            char filename_right[64];
            snprintf(filename_left, sizeof(filename_left), "output/left_%04d.jpg", left.frame_id);
            snprintf(filename_right, sizeof(filename_right), "output/right_%04d.jpg", right.frame_id);
            
            FILE* f_left = fopen(filename_left, "w");
            if (f_left) { 
                fprintf(f_left, "Simulated left image data for frame %d\n", left.frame_id); 
                fclose(f_left); 
            }
            
            FILE* f_right = fopen(filename_right, "w");
            if (f_right) { 
                fprintf(f_right, "Simulated right image data for frame %d\n", right.frame_id); 
                fclose(f_right); 
            }
        } else {
            pthread_mutex_unlock(&sync_mutex);
        }
        
        sleep_for_freq(LOG_FREQ_HZ);
    }
    return NULL;
}

void* robot_state_thread(void* arg) {
    (void)arg;
    double x = 0.0, y = 0.0, theta = 0.0;
    
    while (is_running) {
        // Simulate robot movement
        x += 0.01; 
        y += 0.01; 
        theta += 0.005;
        
        pthread_mutex_lock(&state_mutex);
        current_robot_state.x = x;
        current_robot_state.y = y;
        current_robot_state.theta = theta;
        get_current_time(&current_robot_state.timestamp);
        pthread_mutex_unlock(&state_mutex);
        
        sleep_for_freq(ROBOT_FREQ_HZ);
    }
    return NULL;
}

void* logger_thread(void* arg) {
    (void)arg;
    
    // Save log file in the output directory
    FILE* log_file = fopen("output/robot_state.txt", "w");
    if (!log_file) {
        perror("Failed to open log file");
        return NULL;
    }
    
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
    printf("[LEVEL 1] Starting Robot System Simulation...\n");
    
    // Initialize mutexes and semaphores
    pthread_mutex_init(&camera_mutex, NULL);
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_mutex_init(&state_mutex, NULL);
    
    sem_init(&left_cam_sem, 0, 0);
    sem_init(&right_cam_sem, 0, 0);
    
    // Create threads
    pthread_t threads[6];
    pthread_create(&threads[0], NULL, left_camera_thread, NULL);
    pthread_create(&threads[1], NULL, right_camera_thread, NULL);
    pthread_create(&threads[2], NULL, sync_thread, NULL);
    pthread_create(&threads[3], NULL, image_writer_thread, NULL);
    pthread_create(&threads[4], NULL, robot_state_thread, NULL);
    pthread_create(&threads[5], NULL, logger_thread, NULL);
    
    // Let the system run for 20 seconds
    sleep(20);
    
    // Initiate shutdown
    printf("[LEVEL 1] Stopping system...\n");
    is_running = false;
    
    // Wake up synchronizer thread in case it's waiting on semaphores
    sem_post(&left_cam_sem);
    sem_post(&right_cam_sem);
    
    // Join threads
    for (int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Clean up resources
    pthread_mutex_destroy(&camera_mutex);
    pthread_mutex_destroy(&sync_mutex);
    pthread_mutex_destroy(&state_mutex);
    sem_destroy(&left_cam_sem);
    sem_destroy(&right_cam_sem);
    
    printf("[LEVEL 1] System stopped gracefully.\n");
    return 0;
}

#endif // LEVEL1
