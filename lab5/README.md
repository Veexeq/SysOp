# Lab 5: Process Synchronization utilizing POSIX Semaphores and Shared Memory

## Project Overview
This project implements the classic **Producer-Consumer** (Bounded-Buffer) problem within a multi-process architecture. It leverages Inter-Process Communication (IPC) mechanisms compliant with the **POSIX** standard, specifically utilizing shared memory for high-speed data transfer and semaphores for strict access synchronization. 

The baseline architecture is progressively extended to include priority queuing and a supervisory Manager process designed to mitigate process starvation via an aging algorithm.

## Project Structure
The repository is divided into three developmental stages:
* `zad1/`: Baseline implementation of a synchronized circular buffer.
* `zad2/`: Introduction of dual queues (NORMAL and PRIORITY) with strict consumer prioritization.
* `zad3/`: Complete system architecture featuring the Manager daemon for anti-starvation and system monitoring.

Each task directory follows a standardized internal structure:
* `src/`: C source code and header files (`.c`, `.h`).
* `scripts/`: Automation and testing scripts (e.g., `tmux` environment setup).
* `Makefile`: Build configuration and compilation rules.

## IPC Mechanisms Utilized
1. **POSIX Shared Memory (`shm_open`, `ftruncate`, `mmap`)**: 
   Allocated in the `/dev/shm` filesystem to store the task buffers, providing zero-copy data access across completely independent processes.
2. **POSIX Semaphores (`sem_open`, `sem_wait`, `sem_post`)**:
   * `mutex` (Initial value: 1): Acts as a binary semaphore (lock) ensuring mutually exclusive access to the shared memory critical section.
   * `empty` (Initial value: K): Counting semaphore representing available slots in the buffer. Blocks Producers when the buffer reaches maximum capacity.
   * `full`  (Initial value: 0): Counting semaphore representing queued tasks. Blocks Consumers when the buffer is entirely empty.

## System Components
* **Producer**: Generates randomized alphanumeric strings (10 characters) and inserts them into the shared memory space. In advanced stages (Task 2 & 3), it allocates tasks with a 30% probability to the PRIORITY queue.
* **Consumer**: Retrieves pending tasks from the buffer and simulates processing by printing characters to the standard output with a simulated 0.3s I/O delay. It structurally prioritizes the PRIORITY queue over the NORMAL queue.
* **Manager (Task 3)**: An independent daemon process that executes every 5 seconds. It monitors the IPC state and performs **Task Aging**—transferring the oldest tasks from the NORMAL queue to the PRIORITY queue to prevent indefinite starvation caused by high-priority traffic.
* **Cleaner**: A localized utility application responsible for safely unlinking and releasing persistent POSIX objects (`shm_unlink`, `sem_unlink`) from the kernel.

## Execution Instructions

### 1. Compilation
Navigate to the directory of the chosen task (e.g., `zad3/`) and execute the makefile:
```bash
make all
```

### 2. Test Automation (Recommended)
For Task 3, a shell script is provided to automatically scaffold a 2x2 terminal grid utilizing `tmux`. This will compile the code, clean the environment, and concurrently launch the Manager, Producer, and multiple Consumers.
```bash
./scripts/test_tmux.sh
```
*(Note: Requires the `tmux` multiplexer to be installed on the host system: `sudo apt install tmux`).*

### 3. Manual Execution
To observe the system manually, open multiple terminal instances and launch the binaries in the following recommended order:
```bash
./bin/manager.out
./bin/producer.out
./bin/consumer.out
```

### 4. Resource Cleanup
All IPC clients are designed as daemons and safely intercept the `SIGINT` signal (`Ctrl+C`) to close local file descriptors gracefully. To permanently remove the IPC objects from the operating system's memory, execute:
```bash
./bin/cleaner.out
```

## Error Handling and Concurrency Safety
* **Graceful Shutdown**: Every process implements a `cleanup_handler` bound to `SIGINT`, preventing deadlocks caused by processes terminating while holding the `mutex` lock.
* **Deadlock Prevention**: Strict semaphore acquisition ordering (`empty`/`full` followed by `mutex`) is enforced to prevent circular wait conditions.
* **Data Consistency**: All memory operations and index increments are performed strictly within the bounds of the `mutex` critical section.