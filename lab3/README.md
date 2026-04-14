# Inter-Process Communication: Pipes and FIFOs in C (POSIX) 🚰

This project is a comprehensive exploration of Inter-Process Communication (IPC) mechanisms in the UNIX/Linux environment, specifically focusing on **Unnamed Pipes** and **Named Pipes (FIFOs)**. It demonstrates how to distribute heavy computational workloads across multiple processes, synchronize independent programs, and safely manage system resources and file descriptors.

## 📖 Table of Contents
- [Project Description](#-project-description)
- [Task Structure](#-task-structure)
- [Requirements](#-requirements)
- [Compilation & Execution](#-compilation--execution)
- [Key Concepts](#-key-concepts)

---

## 🚀 Project Description

The core objective of this project is to perform numerical integration of the function $f(x) = 4/(x^2+1)$ over the interval $[0, 1]$ using the rectangle method. The mathematical quirk of this specific integral is that its result naturally converges to the number **$\pi$ (Pi)**, which serves as a great built-in validation for the multithreaded/multiprocess computations.

The project is divided into two distinct architectural approaches:
1.  **Parallel computing using Unnamed Pipes:** Splitting the computational load among a dynamically generated number of related child processes to benchmark multi-core performance.
2.  **Client-Server architecture using Named Pipes:** Establishing two-way communication between completely independent programs running in separate terminal sessions using the file system as an IPC rendezvous point.

---

## 📂 Task Structure

### Task 1: Unnamed Pipes (Parent-Child IPC)
A benchmarking tool that distributes the numerical integration task across $k = 1, 2, ..., n$ concurrent child processes. 
The parent process creates $k$ unnamed pipes (`pipe()`), spawns $k$ children (`fork()`), and assigns a specific mathematical sub-interval to each. The children compute their partial sums and send them back to the parent. The parent aggregates the results, calculates the total execution time using a high-precision monotonic clock (`clock_gettime`), and safely prevents deadlocks by properly closing unused pipe ends.

### Task 2: Named Pipes (Client-Server IPC)
Separation of logic into two independent programs communicating via two unidirectional FIFOs (`/tmp/fifo_request` and `/tmp/fifo_response`):
* **Program 1 (Client):** Prompts the user for the integration bounds and accuracy ($dx$), packs them into an `Interval` C-struct, and writes it to the request FIFO. It then blocks on the response FIFO until the result arrives.
* **Program 2 (Server):** An infinite-loop daemon that creates the FIFOs (`mkfifo`) and listens for incoming structs. Upon receiving data, it performs the integration and sends the resulting `double` back. It includes a custom signal handler (`sigaction`) catching `SIGINT` and `SIGTERM` to gracefully shutdown and clean up (using `unlink`) the FIFO files from the disk.

---

## 💻 Requirements
* Linux / UNIX operating system (POSIX.1-2008 compliant).
* `gcc` compiler.
* `make` build automation tool.

---

## 🛠 Compilation & Execution

The project utilizes a unified `Makefile` that complies both tasks into their respective directories (`zad1/` and `zad2/`). It strictly enforces the **DRY (Don't Repeat Yourself)** principle by sharing the compiled `integrator.o` object file between both tasks.

**Compile everything:**
```bash
make
```

**Clean the project (removes executables and objects):**
```bash
make clean
```

### Running Task 1
Execute the benchmarking program by providing the rectangle width ($dx$) and the maximum number of processes ($n$).
*Note: Use a very small $dx$ (e.g., `0.00000001`) to observe meaningful execution times. Input `0.0` in order to run the program with a default, hardcoded value of $dx$ that should result in approx. 5 second runtime on a single process.*
```bash
./zad1/main.out 0.00000001 10
```

### Running Task 2
Since this involves independent processes, you need **two separate terminal windows**.

**Terminal 1 (Start the Server):**
```bash
./zad2/p2.out
```
*(To gracefully stop the server later, press `Ctrl+C`. It will automatically delete the FIFO files).*

**Terminal 2 (Start the Client):**
```bash
./zad2/p1.out
```
*(Enter the bounds `0` and `1`, and a small $dx$ value when prompted).*

---

## 🧠 Key Concepts
The following system-level programming practices and mechanisms were successfully implemented in this project:
* **Deadlock Prevention:** Strict enforcement of closing unused file descriptors in parent/child pipe architectures to ensure `read()` correctly detects `EOF` (End of File).
* **VLA Avoidance:** Using secure dynamic memory allocation (`malloc`) instead of Variable Length Arrays for pipe arrays to prevent potential Stack Overflow vulnerabilities caused by unpredictable user inputs.
* **FIFO Synchronization:** Leveraging the default blocking behavior of `open()` on named pipes to create a natural "handshake" between the client and server processes without busy-waiting.
* **Graceful Termination:** Catching termination signals (`SIGINT`/`SIGTERM`) to trigger the `unlink()` system call, ensuring that special FIFO files are not left as garbage in the `/tmp/` directory.
* **Binary Struct Transmission:** Sending raw C-struct memory blocks (`sizeof(Interval)`) directly over pipes instead of serializing to strings.