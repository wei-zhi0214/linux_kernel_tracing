#  Process Management – Chapter 8 Labs  
## **Lab: Permission of Process**

###  **Purpose**
The goal of this lab is to **understand and practice process permission and resource limitation** in Linux.  
You will implement a **resource limiter program** (`limiter.c`) that controls how much CPU time, memory, stack, or file descriptors another program can use.

---

###  **Background: Process Resource Limits**
Linux uses **`setrlimit()`** and **`getrlimit()`** system calls to control process-level resource usage.  
Each type of limit is represented by a constant:

| Constant | Description |
|-----------|--------------|
| `RLIMIT_AS` | Maximum virtual memory size (bytes). |
| `RLIMIT_CORE` | Maximum size of a core dump file. |
| `RLIMIT_CPU` | Maximum CPU time (seconds). Sends `SIGXCPU` when exceeded, then `SIGKILL`. |
| `RLIMIT_DATA` | Maximum data segment size. |
| `RLIMIT_FSIZE` | Maximum file size a process may create. Sends `SIGXFSZ` if exceeded. |
| `RLIMIT_LOCKS` | Maximum number of file locks. |
| `RLIMIT_MEMLOCK` | Maximum locked memory (bytes). |
| `RLIMIT_MSGQUEUE` | Maximum bytes in POSIX message queues. |
| `RLIMIT_NICE` | Maximum “nice” value (lowest priority). |
| `RLIMIT_NOFILE` | Maximum number of open file descriptors (`EMFILE` if exceeded). |
| `RLIMIT_NPROC` | Maximum number of processes per user. |
| `RLIMIT_RTPRIO` | Maximum real-time scheduling priority. |
| `RLIMIT_SIGPENDING` | Maximum number of pending signals per user. |
| `RLIMIT_STACK` | Maximum process stack size (bytes). |

---

###  **Lab Requirements**

#### **1️ Implement the resource limiter program – `limiter.c`**
This program launches another executable but **restricts its resource usage** via command-line options.

Supported options:

| Option | Resource | Description |
|---------|-----------|-------------|
| `--as` | `RLIMIT_AS` | Virtual memory limit |
| `--cpu` | `RLIMIT_CPU` | CPU time limit (seconds) |
| `--nofile` | `RLIMIT_NOFILE` | Max open files |
| `--stack` | `RLIMIT_STACK` | Stack size limit |

**Example usage:**
```bash
./limiter --as 256M -- ./memhog 512M
./limiter --cpu 2 -- ./cpuhog
./limiter --nofile 3 -- ./filehog
