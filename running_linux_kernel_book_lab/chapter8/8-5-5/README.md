#  Process Management – Chapter 8 Labs  
## **Lab: Set Up Priority**

###  **Purpose**
Understand and become familiar with **process and thread priority** in Linux,  
and observe how different priorities affect CPU scheduling and throughput under the **CFS (Completely Fair Scheduler)**.

---

###  **Lab Requirement**

1. Use the `setpriority()` function to modify the nice value (priority) of threads.  
2. Program `dyn_pro.c` creates **three worker threads**, all **bound to CPU0**,  
   and sets their priorities respectively to:
   - Thread 1 → nice = **-10**  
   - Thread 2 → nice = **0**  
   - Thread 3 → nice = **10**  
3. Each thread performs a fixed computation loop for **10 seconds**,  
   measuring how many iterations (operations) it completes in that time.

> In Linux, threads are schedulable entities (tasks) with their own TIDs.  
> `setpriority(PRIO_PROCESS, tid, value)` can be used to adjust each thread’s nice value.  
> To minimize scheduling noise, bind all threads to a single CPU with `taskset -c 0`.

---

###  **Execution Example**
```bash
william@William:~/linux-kernel/running_linux_kernel_book_lab/chapter8/8-5-5$ sudo taskset -c 0 ./a.out

=== nice vs throughput (fixed wall time, same CPU) ===
worker 1 start, nice=-10
worker 2 start, nice=0
worker 3 start, nice=10
worker 1 done: wall=10.001 s, iters=446476109, rate=44641337 it/s
worker 2 done: wall=10.000 s, iters=48098073, rate=4809807 it/s
worker 3 done: wall=10.000 s, iters=6357579, rate=635758 it/s
```
###  CFS Scheduling Principle
---
The Completely Fair Scheduler (CFS) does not allocate CPU time directly based on “priority numbers.”
Instead, it converts nice values → weights, and distributes CPU time proportionally to the weight.

| Nice | Weight| 
|------|-------|
| -10  | 9548  |
|   0  | 1024  | 
|  10  | 110   | 

Total Weight = 9548 + 1024 + 110 = 10682 \
---
### Theoretical CPU Share
| Nice | Weight| Theoretical CPU Share|
|------|-------|----------------------|
| -10  | 9548  | 9548 / 10682 ≈ 89.4% |
|   0  | 1024  | 1024 / 10682 ≈ 9.6%  |
|  10  | 110   | 110 / 10682 ≈ 1.0%   |

Lower nice → higher weight → larger CPU share.
---
### Measured Throughput
| Thread | Nice | Iterations/sec |
| ------ | ---- | -------------- |
| 1      | -10  | 44,641,337     |
| 2      | 0    | 4,809,807      |
| 3      | 10   | 635,758        |

Total rate:
TotalRate = 44,641,337 + 4,809,807 + 635,758 = 50,086,902 it/s
Measured CPU shares:
| Nice | Rate   | Share                     |
| ---- | ------ | ------------------------- |
| -10  | 44.64M | 44.64 / 50.09 ≈ **89.1%** |
| 0    | 4.81M  | 4.81 / 50.09 ≈ **9.6%**   |
| 10   | 0.636M | 0.636 / 50.09 ≈ **1.27%** |
