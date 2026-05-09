# Inter-Process Communication: POSIX Message Queues Chat 💬

This project is a practical implementation of Inter-Process Communication (IPC) in the UNIX/Linux environment, specifically utilizing **POSIX Message Queues** (`<mqueue.h>`). It demonstrates how to build a robust, multi-client terminal chat system using a centralized server as a message router, manage asynchronous I/O with process forking, and safely handle system resources.

## 📑 Table of Contents
- [Project Description](#-project-description)
- [Architecture](#-architecture)
- [Requirements](#-requirements)
- [Compilation & Execution](#-compilation--execution)
- [Helper Scripts](#-helper-scripts)
- [Key Concepts](#-key-concepts)

---

## 📝 Project Description

The objective of this project is to create a "hub and spoke" communication system where multiple independent terminal clients can exchange text messages in real-time. Instead of using sockets, the networking is entirely handled by the Linux kernel's virtual file system for POSIX queues (`/dev/mqueue`). 

The system relies on a unified C-struct message protocol, allowing the dynamic transmission of different message types (e.g., initialization requests, server assignments, and standard text broadcasts) using a single, memory-safe data block.

---

## 🏗️ Architecture

The project is divided into two distinct components interacting via IPC:

### 1. The Server (`server.c`)
Acts as the central router. Upon startup, it creates a public, read-only POSIX queue (e.g., `/chat_server_q`). It continuously listens for incoming structures. When receiving an `INIT` message from a new client, it assigns a unique ID and opens a connection to the client's private queue. When receiving a `TEXT` message, it iterates through its registry and broadcasts the payload to all connected clients except the sender.

### 2. The Client (`client.c`)
Upon execution, the client generates a unique private queue based on its PID (e.g., `/chat_client_4012`) and sends an initialization request to the main server queue. After receiving its assigned ID, the client uses the `fork()` system call to split into two concurrent processes:
* **The Child Process:** Blocks on `mq_receive` to constantly listen for incoming broadcasts from the server and print them to the terminal.
* **The Parent Process:** Blocks on `fgets` to read user input from the standard input (`stdin`) and push it to the server's main queue.

---

## 💻 Requirements
* Linux / UNIX operating system (supporting POSIX.1b Real-time extensions).
* `gcc` compiler.
* `make` build automation tool.
* The `-lrt` linker flag (Real-Time library) is required for compiling POSIX queue functions.

---

## 🚀 Compilation & Execution

The project utilizes a structured `Makefile` that compiles source files from the `src/` directory into object files in `obj/` and outputs final executables to the `bin/` directory.

**Compile the entire project:**
```bash
make all
```

**Clean the project (removes executables and objects):**
```bash
make clean
```

### Manual Execution
To test the chat manually, you need to open multiple terminal windows.

**Terminal 1 (Start the Server):**
```bash
./bin/server.out
```

**Terminal 2 & 3 (Start the Clients):**
```bash
./bin/client.out
```
*(Type your messages in the client terminals. To gracefully exit, press `Ctrl+C` in any terminal, which will trigger the cleanup handlers and remove the respective queues).*

---

## 🛠️ Helper Scripts

To streamline testing and ensure system stability, the project includes several Bash utilities:

* **`./tests/demo.sh`** - Runs an automated demonstration of the chat system. It spins up the server and two clients in the background, uses Named Pipes (FIFOs) to simulate user keyboard input, and gracefully terminates everything upon completion.
* **`./tests/util/check_env.sh`** - Scans the system and prints a report of active POSIX queues in `/dev/mqueue`, leftover FIFOs in `/tmp`, and background zombie processes.
* **`./tests/util/cleanup_env.sh`** - A force-cleanup utility that kills any orphaned server/client processes and removes leftover IPC objects from the kernel to restore a clean state.

---

## 🧠 Key Concepts

The following system-level programming practices were successfully implemented in this project:

* **POSIX vs. System V:** Utilizing the modern POSIX standard for queues (named via string identifiers with a leading slash) rather than legacy System V numeric keys.
* **Concurrent I/O via Forking:** Solving the "two-blocking-calls" problem (waiting for keyboard input vs. waiting for network input) in C by duplicating the process via `fork()` instead of using complex `select()`/`poll()` multiplexing or POSIX threads.
* **Kernel Resource Management:** Emphasizing the persistent nature of POSIX queues. Implementing custom signal handlers (`sigaction`/`signal`) to catch `SIGINT` and `SIGTERM`, ensuring `mq_unlink` is always called to prevent kernel memory leaks.
* **Structured Data Transmission:** Defining a universal `ChatMessage` structure with an `enum` header. This allows the system to send raw binary blocks (`sizeof(ChatMessage)`) with `mq_send`, letting the receiver safely cast and interpret the payload type without string parsing.