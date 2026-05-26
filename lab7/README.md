# Iterative TCP Server and Client with HTTP Support

A C-based network application implementing an iterative TCP server with dual-protocol parsing capability (standard HTTP/1.1 and a custom text-based protocol) alongside a dedicated client and a TMUX-driven test automation harness. Built specifically for Linux/WSL2 environments utilizing modern POSIX and jądra system systems mechanisms.

## Project Overview

The project consists of two primary applications communicating over TCP/IPv4:

1. **The Server (`server`)**: 
   - Operates as an iterative network listener on port `9000` across all available interfaces (`INADDR_ANY`).
   - Maintains an internal state variable: `REQUEST_COUNTER` (initialized at 0).
   - Dynamically parses incoming stream data:
     - **HTTP GET/POST Requests**: Increments the counter by 1 and replies with a valid HTTP/1.1 200 OK plain-text response containing the string `"Liczba pobrań strony: %d"`.
     - **Custom "ZADANIE <NUMBER>" Command**: Increments the counter by the specified integer value and responds immediately with the raw string payload (omitting HTTP headers).
   - Implements robust system engineering patterns including graceful shutdown handling (`SIGINT`), socket resource preservation (`SO_REUSEADDR`), and leak prevention (`SOCK_CLOEXEC`).

2. **The Client (`client`)**:
   - A command-line network utility accepting three arguments: Server IPv4, Port, and a Number.
   - Formats the command as `"ZADANIE <NUMBER>"`, establishes a reliable TCP connection, transmits the payload, prints the raw server response, and exits cleanly.

## Key Architectural Highlights

- **Modern Network Extensions**: Utilizes Linux-specific `accept4()` with `SOCK_CLOEXEC` to prevent descriptor leakage across process forks.
- **Graceful Shutdown Hook**: Replaces standard unsafe signal routines with a POSIX-compliant `sigaction()` structure. Interrupted blocking calls (returning `EINTR`) are gracefully intercepted to allow systematic garbage collection of network descriptors.
- **Signal Resilience**: Uses `MSG_NOSIGNAL` on data transmissions (`send()`) to mask `SIGPIPE` generation, securing the process against unexpected client socket dropouts.
- **Byte Order Integrity**: Enforces structural conversion between Host Byte Order (Little-Endian) and Network Byte Order (Big-Endian) using `htons()` and `htonl()` wrappers.

## Project Structure

```text
.
├── Makefile                # Strict compilation definition file
├── bin/                    # Compiled production binaries (server, client)
├── obj/                    # Temporary translation object files (.o)
├── include/                # Universal project header declarations
├── scripts/                # Production-ready shell scripts
│   └── test_automation.sh  # Complete TMUX automation deployment harness
└── src/                    # Implementation source files
    ├── client/
    │   └── main.c          # Client source entry point
    └── server/
        └── main.c          # Server source entry point
```

## Prerequisites

* **Operating System**: Linux kernel 2.6.28+ or WSL2 (Windows Subsystem for Linux).
* **Toolchain**: `gcc` compiler supporting the ISO C11 standard and `GNU make`.
* **System Diagnostics**: `tmux` (required for automated sequence monitoring), `iproute2` (`ss`), and `netcat` (`nc`).

To install dependencies on Debian/Ubuntu-based environments:

```bash
sudo apt update && sudo apt install build-essential tmux iproute2 netcat-openbsd -y
```

## Compilation

The project uses `GNU Make` with strict error policing (`-Wall -Wextra -Werror`). Any compiler warnings will abort the compilation pipeline to enforce code reliability.

To compile both target binaries:

```bash
make
```

To purge object layers and binaries:

```bash
make clean
```

## Execution and Usage

### Manual Operations

1. **Initialize the Listener Node**:
```bash
./bin/server
```


2. **Execute a Custom Client Call**:
```bash
./bin/client 127.0.0.1 9000 42
```



### Interface Diagnostics

* **Web Browser Validation**: Point any standard web browser to `http://127.0.0.1:9000`. The browser will submit a speculative `GET` sequence, advancing the server state machine and outputting the counter.
* **Raw Diagnostic Injection via Netcat**:
```bash
echo -n "ZADANIE 10" | nc 127.0.0.1 9000
```


* **Socket Allocation Tracking**: Check socket listening states via `ss`:
```bash
ss -tulpn | grep 9000
```



## Automated Evaluation Environment

The repository includes an automated integration test suite located within `scripts/test_automation.sh`.

This shell script dynamically:

1. Validates the filesystem topology and existence of compiled targets.
2. Initializes a detached background `tmux` instance.
3. Automatically maps directory contexts to mitigate jądra system defaults.
4. Spawns the server block on Pane 0, inserts a calculated polling threshold, splits the matrix horizontally, and queues up programmatic `client` executions on Pane 1.
5. Injects automated text sequences, handles real-time visual output, signals a system `SIGINT` interruption vector to trigger the server's internal resource destructor, and attaches to the live buffer.

To execute the automation framework:

```bash
chmod +x scripts/test_automation.sh
./scripts/test_automation.sh
```

## Memory Allocation Profile

The application has been audited under binary translation tools (`Valgrind Memcheck`) to confirm absence of heap fragmentation, descriptor pollution, or memory leaks.

```bash
valgrind --leak-check=full ./bin/server
```

Expected execution signature: `All heap blocks were freed -- no leaks are possible`.