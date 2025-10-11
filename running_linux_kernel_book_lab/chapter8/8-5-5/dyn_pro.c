#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
static double now_wall(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void pin_to_cpu0(void){
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if (sched_setaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
    }
}

void cpu_intensive_work(int worker_id, int priority, int duration_seconds) {
    pin_to_cpu0(); // 讓大家都在 CPU0 競爭

    if (setpriority(PRIO_PROCESS, 0, priority) != 0) {
        fprintf(stderr, "worker %d setpriority(%d) failed: %s\n",
                worker_id, priority, strerror(errno));
    }

    errno = 0;
    int cur = getpriority(PRIO_PROCESS, 0);
    if (errno == 0)
        printf("worker %d start, nice=%d\n", worker_id, cur);

    double t0 = now_wall();
    double deadline = t0 + duration_seconds;

    volatile unsigned long long sum = 0;
    unsigned long long iters = 0;

    while (now_wall() < deadline) {
        // 純計算，避免 syscalls/IO 影響
        sum += iters;
        iters++;
    }

    double t1 = now_wall();
    printf("worker %d done: wall=%.3f s, iters=%llu, rate=%.0f it/s\n",
           worker_id, t1 - t0, iters, iters / (t1 - t0));
    _exit(0);
}

int main(void){
    pid_t ps[3];
    int prios[3] = {-10, 0, 10};     // -10 需要 root；沒權限會失敗但仍可觀察 0 vs 10
    int secs = 10;

    puts("=== nice vs throughput (fixed wall time, same CPU) ===");

    for (int i = 0; i < 3; i++) {
        pid_t c = fork();
        if (c == 0) {
            cpu_intensive_work(i+1, prios[i], secs);
        } else if (c > 0) {
            ps[i] = c;
        } else {
            perror("fork");
            return 1;
        }
    }

    for (int i = 0; i < 3; i++) {
        waitpid(ps[i], NULL, 0);
    }
    return 0;
}

