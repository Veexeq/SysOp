# Lab 1: Process Management in UNIX

This directory contains solutions to introductory tasks regarding process management in a UNIX environment, heavily relying on Linux kernel system calls.

## 📝 Theory and Core Mechanisms

The tasks in this laboratory are based on the following system-level concepts:

1.  **Process Identification:** Utilizing `getpid()` and `getppid()` from the `<unistd.h>` library to retrieve the current process ID and its parent's ID.
2.  **Process Creation (`fork`):** Using the `fork()` system call to clone processes. Remember that `fork()` returns `0` in the child process, the child's PID in the parent process, and `-1` in case of an error.
3.  **Resource Management (Zombies and Orphans):** Applying `wait()` and `waitpid()` from the `<sys/wait.h>` library to reap the exit statuses of child processes. This prevents the creation of *Zombie* processes (which appear with a `Z` status in tools like `top`).
4.  **Executing New Programs (`exec*`):** Using the `exec` family of functions to load a new program into the address space of a process created by `fork()`. The `exec` call preserves the current PID and open file descriptors.
5.  **File Descriptors and I/O:** Performing basic operations on file descriptors using low-level Linux kernel system calls: `open()`, `read()`, `write()`, and `close()`.

## 💻 Task Structure

The following tasks have been implemented in this directory:

*   **Task 1:** Process tree. The program creates multiple generations of child processes using `fork()`, with each process printing its own PID and its parent's PPID.
*   **Task 2:** The `exec*` family. Using the `execlp` function to execute a system command (e.g., `ls -a -l`) within a newly created child process.
*   **Task 3:** Input/Output (I/O) operations. A program that creates a child process where both the parent and child share open file descriptors and alternate writing data to them.

## 🛠️ Compilation and Execution

The compilation method depends on the complexity of the task. Navigate to the specific task directory before compiling.

### Task 1 (Manual Compilation)
This task consists of standalone C files. You can manually compile either the standard `fork` version or the `vfork` version using `gcc`:

```bash
cd zad1

# Compile and run the standard fork version
gcc -Wall main.c -o main.out
./main.out

# OR compile and run the vfork version
gcc -Wall main_vfork.c -o main_vfork.out
./main_vfork.out
```

### Tasks 2 & 3 (Using Makefile)
Tasks 2 and 3 involve multiple source files (e.g., `main.c` and `child.c` running as separate programs). A `Makefile` is provided in each of these directories to automate the build process.

```bash
# Example for Task 2 (applies to Task 3 as well)
cd zad2

# Build the project using make
make

# Run the main executable
./main.out
```