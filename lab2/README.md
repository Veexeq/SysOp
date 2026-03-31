# Signals, IPC, and Libraries in C (POSIX) 🐧

This project is a comprehensive collection of C programs demonstrating advanced operating system mechanisms from the UNIX/Linux family. It focuses on POSIX signal handling, Inter-Process Communication (IPC), creating modular libraries, and automating the build and test processes.

## 📖 Table of Contents
- [Project Description](#-project-description)
- [Task Structure](#-task-structure)
- [Requirements](#-requirements)
- [Compilation & Execution (Task 3)](#-compilation--execution-task-3)
- [Automated Testing (CI)](#-automated-testing-ci)
- [Key Concepts](#-key-concepts)

---

## 🚀 Project Description

The project is divided into three stages (tasks), each expanding the previous one with new system-level concepts: from basic signal interception, through parent-child communication via signal queues, to full modularization using dynamically loaded libraries (`dlopen`).

The main program allows testing 4 different system reactions to the `SIGUSR1` signal:
1. `default` - Default system behavior (terminates the process).
2. `ignore` - Ignores the signal completely (`SIG_IGN`).
3. `handle` - Intercepts the signal with a custom function and safely prints a message (using async-signal-safe `write`).
4. `mask` - Blocks (masks) the signal, checks for its presence in the pending queue (`sigpending`), and unblocks it later.

---

## 📂 Task Structure

### Task 1: Signal Basics
A monolithic program executing a 20-second loop. At the 5th and 15th seconds, the program sends the `SIGUSR1` signal to itself. At the 10th second, it checks the pending signals queue and unblocks the signal if it was previously masked.

### Task 2: Processes and IPC
Separation of logic into two independent processes. The `main` program (parent) creates a new process (`fork`) and replaces its image (`exec`) with the `child` program. The parent uses `sigqueue` to send a `SIGUSR2` signal containing a payload (the chosen operation mode) to the child. This triggers the appropriate mask and handler configuration in the child process.

### Task 3: Libraries, Modularization, and Makefile
Refactoring the signal handling functions into separate modules (`sig_default.c`, `sig_mask.c`, etc.). Implementation of a `Makefile` that builds the project in three different architectures:
* **Static** (`.a`) - Compiled directly into the executable.
* **Shared** (`.so`) - Linked at compile time (utilizing `rpath`).
* **Dynamically Loaded** - Using preprocessor directives (`-DUSE_DYNAMIC`) and the `<dlfcn.h>` interface to manually open the `.so` library and resolve function symbols at runtime.

---

## 💻 Requirements
* Linux / UNIX operating system (POSIX.1-2008 compliant).
* `gcc` compiler.
* `make` build automation tool.
* `bash` shell (for running the automated test suite).

---

## 🛠 Compilation & Execution (Task 3)

The project utilizes a smart `Makefile` that compiles only the modified files and automatically injects appropriate file paths via the `CHILD_PATH` macro.

**Clean the project directory:**
```bash
make clean
```

**Compile and run the Static version:**
```bash
make static
./main.out <default|mask|ignore|handle>
```

**Compile and run the Shared version:**
```bash
make shared
./main.out <default|mask|ignore|handle>
```

**Compile and run the Dynamically Loaded (dlopen) version:**
```bash
make dynamic
./main.out <default|mask|ignore|handle>
```

*(Note: You can simply run `make` without arguments to build all 3 variants simultaneously).*

---

## 🧪 Automated Testing (CI)

The project includes a bash script `test.sh` that acts as a simple Continuous Integration system. The script:
1. Sequentially builds all 3 library variants.
2. Runs tests for all 4 operation modes **concurrently in the background**, resolving stream buffering issues using `stdbuf`.
3. Parses the logs using regular expressions (`grep`) to evaluate assertions, outputting a clear `[PASS]` or `[FAIL]`.
4. Manages execution logs in a dedicated, temporary `logs/` directory.

**Running the test suite:**
```bash
chmod +x test.sh  # (Required only once)
./test.sh
```

---

## 🧠 Key Concepts
The following system-level programming practices and mechanisms were successfully implemented in this project:
* Mitigating **Race Conditions** between `exec` calls and signal delivery through synchronization and process sleeping.
* **Async-Signal-Safety**: Replacing standard `printf` functions with low-level `write` system calls inside signal handlers, combined with manual buffer formatting.
* Understanding kernel-level behavior regarding the restoration of signal masks upon returning from a handler.
* Configuring the `rpath` (`$ORIGIN`) for proper local shared object resolution without polluting system directories.