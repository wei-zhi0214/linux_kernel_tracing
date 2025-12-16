#  Memory Management – Chapter 8 Labs
## **Lab: Check system memory information **

###  **Purpose**
1. know memory management using mointor tool 

---

###  **Lab Requirement**

1. familier with "top" and "vmstat" command

---
### top 

```
top - 05:21:00 up 49 min,  1 user,  load average: 0.00, 0.00, 0.00
Tasks:  33 total,   1 running,  31 sleeping,   1 stopped,   0 zombie
%Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :  15706.2 total,  15071.4 free,    403.7 used,    231.1 buff/cache
MiB Swap:   4096.0 total,   4096.0 free,      0.0 used.  15080.6 avail Mem

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
      1 root      20   0  100248  10748   8188 S   0.0   0.1   0:00.53 systemd
      2 root      20   0    3060   1664   1664 S   0.0   0.0   0:00.00 init-systemd(Ub
      6 root      20   0    3076   1792   1792 S   0.0   0.0   0:00.00 init
     60 root      19  -1   47812  14852  13956 S   0.0   0.1   0:00.25 systemd-journal
     82 root      20   0   22972   6016   4608 S   0.0   0.0   0:00.38 systemd-udevd
    127 systemd+  20   0   26200  14200   9216 S   0.0   0.1   0:00.10 systemd-resolve
    134 systemd+  20   0   89364   7040   6272 S   0.0   0.0   0:00.16 systemd-timesyn
    199 root      20   0    4308   2688   2432 S   0.0   0.0   0:00.00 cron
    201 message+  20   0    8584   4608   4096 S   0.0   0.0   0:00.12 dbus-daemon
    206 root      20   0   30176  18432   9856 S   0.0   0.1   0:00.09 networkd-dispat
    207 syslog    20   0  222404   5120   4352 S   0.0   0.0   0:00.03 rsyslogd
    211 root      20   0   15328   7424   6528 S   0.0   0.0   0:00.09 systemd-logind
    230 root      20   0    3240   2176   2048 S   0.0   0.0   0:00.00 agetty
    234 root      20   0    3196   2048   2048 S   0.0   0.0   0:00.00 agetty
    241 root      20   0  107160  21120  13056 S   0.0   0.1   0:00.06 unattended-upgr
    310 root      20   0    3060    896    896 S   0.0   0.0   0:00.00 SessionLeader
    311 root      20   0    3076   1024   1024 S   0.0   0.0   0:00.40 Relay(312)
    312 william   20   0    6388   5248   3456 S   0.0   0.0   0:00.21 bash
    313 root      20   0    7520   4736   3968 S   0.0   0.0   0:00.00 login
    402 william   20   0   16928   9344   7808 S   0.0   0.1   0:00.05 systemd
    403 william   20   0  103300   5080   1664 S   0.0   0.0   0:00.00 (sd-pam)
    408 william   20   0    6232   5120   3328 S   0.0   0.0   0:00.01 bash
```

|   Item    | Description                                               |
| :-------: | :-------------------------------------------------------- |
|    PID    | Shows task’s unique process id.|
|    USER   | User name of owner of task.|
|    PR     | The process’s priority. The lower the number, the higher the priority|
|    NI     | Represents a Nice Value of task. A Negative nice value implies higher priority, and positive Nice value means lower priority.|
| VIRT      | Total virtual memory used by the task |
| RES       | How much physical RAM the process is using, measured in kilobytes|
| SHR       | Represents the Shared Memory size (kb) used by a task|
| %CPU      | Represents the CPU usage|
| %MEM      | Shows the Memory usage of task|
| TIME+     | CPU Time, the same as ‘TIME’, but reflecting more granularity through hundredths of a second|
| COMMAND   | The name of the command that started the process|

MiB Memory:
| total |Total amount of physical memory (RAM) available in MiB|
| used  |Amount of RAM currently in use by processes and the kernel|
| free  |Amount of RAM not being used at all|
|buff/cache|Amount of memory used for buffering data and caching filesystems|

Mib Swap:
|total|Total amount of swap space available in MiB|
|used|Amount of swap space currently in use|
|free|Amount of swap space that is not being used|
|available|Estimate of how much memory is available for starting new applications without swapping|

ref:https://www.geeksforgeeks.org/linux-unix/top-command-in-linux-with-examples/

### vmstat 
```
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 0  0      0 15430416  10948 229348    0    0     7    12    9    8  0  0 100  0  0
```

## procs
| Field | Description |
|------|------------|
| r | Number of runnable processes waiting for CPU |
| b | Number of processes in uninterruptible sleep (usually I/O wait) |

## memory
| Field | Description |
|------|------------|
| swpd | Amount of virtual memory used (swap) |
| free | Amount of idle physical memory |
| buff | Memory used as buffer cache |
| cache | Memory used as page cache |

## swap
| Field | Description |
|------|------------|
| si | Swap in (memory swapped from disk) |
| so | Swap out (memory swapped to disk) |

## io
| Field | Description |
|------|------------|
| bi | Blocks received from a block device per second |
| bo | Blocks sent to a block device per second |

## system
| Field | Description |
|------|------------|
| in | Number of interrupts per second |
| cs | Number of context switches per second |

## cpu
| Field | Description |
|------|------------|
| us | Time spent running user processes |
| sy | Time spent running kernel code |
| id | Idle CPU time |
| wa | Time spent waiting for I/O |
| st | Time stolen from this VM by the hypervisor |

ref:https://blog.csdn.net/weixin_45277068/article/details/148103870
