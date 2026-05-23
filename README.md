# Operating Systems @ AGH UST 🐧

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.linux.org/)
[![Bash](https://img.shields.io/badge/Bash-4EAA25?style=for-the-badge&logo=gnu-bash&logoColor=white)](https://www.gnu.org/software/bash/)
[![Status: Work in Progress](https://img.shields.io/badge/Status-Work%20in%20Progress-yellow?style=for-the-badge)]()

A comprehensive collection of solutions and projects for the **Operating Systems** laboratory course, part of the Computer Science curriculum at the Faculty of Computer Science, AGH University of Science and Technology in Kraków.

The projects focus on low-level system programming in UNIX/Linux environments, utilizing the C programming language, strict adherence to the POSIX.1 standard, and robust build automation and testing methodologies.

## 📂 Repository Structure

| Directory | Topic | Description |
| :--- | :--- | :--- |
| [`lab1/`](./lab1) | Processes and Identifiers | Process creation (`fork`), `exec` functions, child process management (`wait`), and low-level file descriptors. |
| [`lab2/`](./lab2) | Signals, IPC & Libraries | Advanced POSIX signal handling (`sigaction`, `sigprocmask`), Inter-Process Communication via `sigqueue`, modularization with Static/Shared/Dynamic libraries (`dlopen`), and automated CI testing. |
| [`lab3/`](./lab3) | Pipes and FIFOs (IPC) | Inter-Process Communication using Unnamed Pipes (`pipe`) and Named Pipes/FIFOs (`mkfifo`), deadlock prevention, and client-server process synchronization. |
| [`lab4/`](./lab4) | POSIX Message Queues (IPC) | A "hub and spoke" communication system where multiple independent terminal clients exchange text messages in real-time using `/dev/mqueue`. Features asynchronous I/O managed by duplicating processes via `fork()`. |
| [`lab5/`](./lab5) | POSIX Semaphores & Shared Memory | Implementation of the classic Producer-Consumer problem within a multi-process architecture. Leverages POSIX shared memory for zero-copy data transfer and POSIX semaphores for strict access synchronization, complete with a Manager daemon mitigating starvation via an aging algorithm. |
| [`lab6/`](./lab6) | POSIX Threads, RTOS scheduling | A comprehensive, multithreaded simulation of a mobile robot's internal operating system. POSIX threads (pthreads), complex inter-thread communication, synchronization mechanisms, and Real-Time Operating System (RTOS) scheduling. |

## ⚙️ System Requirements

To successfully compile and run the code in this repository, a POSIX-compliant environment is required (e.g., native Linux distribution, WSL on Windows, or macOS).

Required tools:
* `gcc` (GNU Compiler Collection).
* `make` (required for multi-file projects and automated builds).
* `bash` (for executing the automated test suites and environment cleanup scripts).
* `tmux` (optional, heavily recommended for simulating multi-process execution in advanced IPC labs).

## 🚀 Compilation & Execution

Each laboratory folder contains a dedicated `README.md` file with highly specific compilation instructions, architectural details, and testing methodologies. 

**Unified Build Process:**
From Lab 2 onwards, all projects utilize a structured `Makefile` system. You can generally compile an entire laboratory module by navigating to its directory and executing:
```bash
make all
```

**Automated Testing & Cleanup:**
Advanced labs dealing with persistent kernel resources (like FIFOs, Message Queues, and Shared Memory) include dedicated Bash utilities located in the `scripts/` or `tests/` directories. These scripts facilitate multi-terminal demonstrations and ensure safe environment cleanup:
```bash
# Example: Launching the automated tmux simulation for Lab 5
./scripts/test_tmux.sh
```

---
*Created by Wiktor Trybus*