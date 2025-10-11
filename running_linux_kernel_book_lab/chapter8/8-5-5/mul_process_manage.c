// multiprocess_priority_time.c
#define _GNU_SOURCE
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

/* 取得單一 process 的 user+sys CPU time（秒） */
static double proc_cpu_time_sec(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    double ut = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
    double st = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    return ut + st;
}

/* 取得單調時鐘（牆鐘） */
static double now_wall(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/**
 * 工作進程函數：設定優先級 + 計時 + 模擬負載
 */
void worker_process(int worker_id, int priority) {
    pid_t pid = getpid();

    double wall_start = now_wall();
    double cpu_start  = proc_cpu_time_sec();

    printf("工作進程 %d 啟動 (PID: %d)\n", worker_id, pid);

    // 設定進程優先級（nice 值）
    if (setpriority(PRIO_PROCESS, 0, priority) == 0) {
        printf("工作進程 %d 優先級設為 %d\n", worker_id, priority);
    } else {
        printf("工作進程 %d 優先級設置失敗: %s\n", worker_id, strerror(errno));
    }

    // 顯示目前優先級
    errno = 0;
    int current_prio = getpriority(PRIO_PROCESS, 0);
    if (errno == 0) {
        printf("工作進程 %d 當前優先級: %d\n", worker_id, current_prio);
    } else {
        printf("工作進程 %d 無法取得優先級: %s\n", worker_id, strerror(errno));
    }

    // 模擬工作負載（計算密集）
    volatile long long sum = 0;
    const long long N = 1000000000LL;
    for (long long i = 0; i < N; i++) {
        sum += i;
        if (i % 100000000LL == 0) {
            printf("工作進程 %d: 進度 %lld%%\n", worker_id, i / 10000000LL);
        }
    }

    double wall_end = now_wall();
    double cpu_end  = proc_cpu_time_sec();

    printf("工作進程 %d 完成，計算結果: %lld\n", worker_id, sum);
    printf("工作進程 %d 時間統計: wall=%.3f s, cpu(user+sys)=%.3f s\n",
           worker_id, wall_end - wall_start, cpu_end - cpu_start);

    exit(0);
}

/**
 * 多進程優先級管理 + 計時示範（父行程也計總時間）
 */
int demo_multiprocess_priority() {
    pid_t workers[3];
    int priorities[] = {-5, 0, 10};  // 高 / 中 / 低（注意：-5 需要 root 或 CAP_SYS_NICE）
    int worker_count = 3;

    printf("=== 多進程優先級管理 + 執行時間演示 ===\n");
    printf("父進程 PID: %d, 當前優先級: %d\n",
           getpid(), getpriority(PRIO_PROCESS, 0));
    printf("\n");

    double parent_wall_start = now_wall();

    // 建立子進程
    printf("建立 %d 個工作進程:\n", worker_count);
    for (int i = 0; i < worker_count; i++) {
        workers[i] = fork();
        if (workers[i] == 0) {
            // 子進程
            worker_process(i + 1, priorities[i]);
        } else if (workers[i] > 0) {
            // 父進程
            printf("  建立工作進程 %d: PID=%d, 設定優先級=%d\n",
                   i + 1, workers[i], priorities[i]);
        } else {
            perror("建立工作進程失敗");
            // 清理已建立的子進程
            for (int j = 0; j < i; j++) kill(workers[j], SIGKILL);
            return -1;
        }
    }

    // 顯示各子進程（建立後）優先級（僅資訊性，實際值以子進程內設定為準）
    printf("\n建立後查詢子進程優先級（可能因權限而與預期不同）:\n");
    for (int i = 0; i < worker_count; i++) {
        errno = 0;
        int prio = getpriority(PRIO_PROCESS, workers[i]);
        if (errno == 0) {
            printf("  進程 %d (PID=%d): 優先級=%d\n", i + 1, workers[i], prio);
        } else {
            printf("  進程 %d (PID=%d): 無法取得優先級 (%s)\n",
                   i + 1, workers[i], strerror(errno));
        }
    }

    // 等待所有子進程完成，順便印出每個子進程的 rusage（父視角）
    printf("\n等待工作進程完成:\n");
    int finished = 0;
    while (finished < worker_count) {
        int status = 0;
        struct rusage ru_child;
        // 使用 wait4 可取得該子進程的 user/sys CPU 時間（父視角）
        pid_t done = wait4(-1, &status, 0, &ru_child);
        if (done > 0) {
            finished++;
            double ut = ru_child.ru_utime.tv_sec + ru_child.ru_utime.tv_usec / 1e6;
            double st = ru_child.ru_stime.tv_sec + ru_child.ru_stime.tv_usec / 1e6;
            printf("  子進程 (PID=%d) 完成，exit=%d, cpu(user)=%.3f s, cpu(sys)=%.3f s, cpu(total)=%.3f s\n",
                   done, WIFEXITED(status) ? WEXITSTATUS(status) : -1,
                   ut, st, ut + st);
        } else if (done == -1 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    double parent_wall_end = now_wall();
    printf("\n所有工作進程已完成，父進程總 wall 時間：%.3f s\n",
           parent_wall_end - parent_wall_start);
    return 0;
}

int main(void) {
    return demo_multiprocess_priority();
}

