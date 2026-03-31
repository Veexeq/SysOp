# Operating Systems - AGH UST 🐧

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.linux.org/)
[![Bash](https://img.shields.io/badge/Bash-4EAA25?style=for-the-badge&logo=gnu-bash&logoColor=white)](https://www.gnu.org/software/bash/)
[![Status: Work in Progress](https://img.shields.io/badge/Status-Work%20in%20Progress-yellow?style=for-the-badge)]()

A collection of solutions and projects for the **Operating Systems** laboratory course, part of the Computer Science curriculum at the Faculty of Computer Science, AGH University of Science and Technology in Kraków.

The projects focus on low-level programming in UNIX/Linux environments, utilizing the C programming language, the POSIX standard, and automated testing via Bash scripting.

## 📂 Repository Structure

| Directory | Topic | Description |
| :--- | :--- | :--- |
| [`lab1/`](./lab1) | Processes and Identifiers | Process creation (`fork`), `exec` functions, child process management (`wait`), and file descriptors. |
| [`lab2/`](./lab2) | Signals, IPC & Libraries | Advanced POSIX signal handling (`sigaction`, `sigprocmask`), Inter-Process Communication via `sigqueue`, modularization with Static/Shared/Dynamic libraries (`dlopen`), and automated CI testing. |
## ⚙️ System Requirements

To successfully compile and run the code in this repository, a POSIX-compliant environment is required (e.g., native Linux distribution, WSL on Windows, or macOS).

Required tools:
* `gcc` (GNU Compiler Collection)
* `make` (required for multi-file projects and automated builds)
* `bash` (for running the automated test suites)

## 🚀 How to Run the Code

Each laboratory folder contains a dedicated `README.md` file with specific compilation and testing instructions. 

**For simple, single-file programs (e.g., Lab 1):**
```bash
gcc -Wall -o program_name file_name.c
./program_name
```

**For modular projects with build automation (e.g., Lab 2):**
Projects utilizing multiple modules and dynamic libraries include a `Makefile`. You can build specific architectures or use the automated test suites:
```bash
# Build a specific architecture (static, shared, dynamic)
make dynamic

# Run the compiled executable
./main.out <arguments>

# Run the automated Continuous Integration (CI) test suite
./test.sh
```

---
*Created by Wiktor Trybus*