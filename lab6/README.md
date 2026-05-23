# Real-Time Mobile Robot System Simulation

A comprehensive, multithreaded simulation of a mobile robot's internal operating system written in C. This project demonstrates advanced system programming concepts, including POSIX threads (pthreads), complex inter-thread communication, synchronization mechanisms, and Real-Time Operating System (RTOS) scheduling.

## 📌 Project Overview
In modern robotics (similar to the ROS architecture), various sensors operate at different frequencies. This project simulates a core system where two cameras generate image frames at 25 Hz, while the robot's odometry/state updates at 100 Hz. The system handles the asynchronous nature of these sensors, synchronizes stereo camera frames, and safely logs data to a simulated disk at 10 Hz—all without race conditions or data corruption.

## 🚀 Key Features & Technologies
- **Multithreading:** Concurrent execution of up to 7 independent threads (`pthread`).
- **Synchronization:** Utilization of Mutexes (`pthread_mutex_t`) and Semaphores (`sem_t`) to prevent race conditions.
- **Advanced IPC (Inter-Process Communication):** 
  - Implementation of **Circular Buffers (FIFO)** to queue sensory data and prevent data loss.
  - **Condition Variables (`pthread_cond_t`)** to completely eliminate busy-waiting.
- **Real-Time Scheduling:** Implementation of `SCHED_FIFO` policy for critical threads.
- **Lock-Free Operations:** Usage of C11 Atomics (`stdatomic.h`) for high-performance, lock-free system monitoring.
- **Graceful Shutdown:** Interception of `SIGINT` (`CTRL+C`) for safe resource deallocation and memory cleanup (`pthread_join`, `pthread_mutex_destroy`).

## 🏗️ System Architecture
The system consists of the following concurrent threads:
1. **Left Camera Thread (25 Hz):** Generates image frames and timestamps.
2. **Right Camera Thread (25 Hz):** Generates image frames and timestamps.
3. **Synchronizer Thread:** Evaluates timestamps and pairs left/right frames if the time difference is `< 20 ms`.
4. **Image Writer Thread (10 Hz):** Pulls paired frames and simulates disk I/O by generating `.jpg` (text) files.
5. **Robot State Thread (100 Hz):** High-priority thread simulating the robot's spatial movement (X, Y, Theta).
6. **Logger Thread (10 Hz):** Periodically captures and logs the current robot state to `robot_state.txt`.
7. **Watchdog Thread (1 Hz):** Monitors the frequency of all threads using atomic variables to ensure strict Real-Time deadlines are met.

## 📁 Directory Structure
```text
projekt_os/
├── bin/                 # Compiled executable binaries (.out)
├── include/             # Header files (common.h)
├── obj/                 # Compiled object files (.o)
├── output/              # Generated logs, text files, and simulated .jpgs
├── scripts/             # Bash scripts for automated building and testing
├── src/                 # Source code (.c files)
├── Makefile             # Build automation
└── .gitignore
```

## 🛠️ Project Progression (Levels)

The project is built iteratively in three distinct levels, compiling different sets of features.

### Level 1: Basic Synchronization (`program1.c`)

* Establishes the 6 core threads.
* Implements basic synchronization using standard Mutexes and Semaphores.
* Runs for a fixed duration of 20 seconds before shutting down automatically.

### Level 2: Circular Buffers & Conditional Variables (`program2.c`)

* Introduces robust **64-element Circular Buffers (FIFO)** for frame handling.
* Replaces raw sleep routines in reader threads with **Condition Variables**, ensuring microsecond-level reaction times when data arrives.
* Introduces **Graceful Shutdown** via `SIGINT` (CTRL+C) handling.

### Level 3: RTOS & Lock-Free Monitoring (`program3.c`)

* Upgrades to **Real-Time Scheduling (`SCHED_FIFO`)** granting hardware-level priorities to critical threads (Robot State & Watchdog).
* Implements lock-free counters using `<stdatomic.h>`.
* Generates a final performance report upon shutdown.

## ⚙️ Compilation & Execution

### Prerequisites

* Linux / Windows Subsystem for Linux (WSL)
* GCC Compiler
* GNU Make

### Building the Project

You can build specific levels using the provided Makefile:

```bash
make clean
make level1   # Builds bin/level1.out
make level2   # Builds bin/level2.out
make level3   # Builds bin/level3.out
```

### Automated Testing via Bash Scripts

The easiest way to observe the system is by using the provided bash scripts. They automatically compile the code, run the simulation, simulate keyboard interruptions (`CTRL+C`), and print a summary of the output directory.

```bash
./scripts/run_level1.sh
./scripts/run_level2.sh
./scripts/run_level3.sh
```

### ⚠️ Note on Real-Time Execution (Level 3)

In Level 3, the program attempts to use the `SCHED_FIFO` real-time scheduling policy. On standard Linux configurations, unprivileged users are not allowed to set real-time priorities (to prevent system lockups).

If you run `./scripts/run_level3.sh` normally, the system will fall back to default scheduling and display a warning.
To observe true RTOS capabilities without dropped frames or missed deadlines, execute the script with `sudo`:

```bash
sudo ./scripts/run_level3.sh
```

## 🧹 Cleanup

To remove all compiled binaries, object files, and generated output logs, simply run:

```bash
make clean
```