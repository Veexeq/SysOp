# Operating Systems - AGH UST 🐧

[![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.linux.org/)
[![Status: Work in Progress](https://img.shields.io/badge/Status-Work%20in%20Progress-yellow?style=for-the-badge)]()

A collection of solutions and projects for the **Operating Systems** laboratory course, part of the Computer Science curriculum at the Faculty of Computer Science, AGH University of Science and Technology in Kraków.

The projects focus on low-level programming in UNIX/Linux environments, utilizing the C programming language and the POSIX standard.

## 📂 Repository Structure

| Directory | Topic | Description |
| :--- | :--- | :--- |
| [`lab1/`](./lab1) | Processes and Identifiers | Process creation (`fork`), `exec` functions, child process management (`wait`), and file descriptors. |
<!-- Future labs will be added here -->

## ⚙️ System Requirements

To successfully compile and run the code in this repository, a POSIX-compliant environment is required (e.g., native Linux distribution, WSL on Windows, or macOS).

Required tools:
* `gcc` (GNU Compiler Collection)
* `make` (optional, but recommended for building)

## 🚀 How to Run the Code

Each laboratory folder contains a dedicated `README.md` file with specific compilation instructions. 
The general compilation pattern for single C files is:

```bash
gcc -Wall -o program_name file_name.c
./program_name
```

---
*Created by Wiktor Trybus*