// lab_8_5_priority.c
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

static int get_nice_checked(int which, id_t who, int *out) {
    errno = 0;
    int val = getpriority(which, who);  // -20..19
    if (errno != 0) return -1;
    *out = val;
    return 0;
}

int main(int argc, char **argv) {
    // 預設操作「自己這個行程」
    id_t target = getpid();
    if (argc >= 2) {
        // 也可指定 PID： ./a.out 1234
        target = (id_t)strtol(argv[1], NULL, 10);
    }

    printf("[info] target pid = %d\n", (int)target);

    // 先讀取目前 nice
    int cur;
    if (get_nice_checked(PRIO_PROCESS, target, &cur) == 0)
        printf("[info] current nice = %d\n", cur);
    else
        perror("getpriority(init)");

    printf("[info] loop set nice from -20 to 19 ...\n");

    for (int n = -20; n <= 19; n++) {
        // 設定
        if (setpriority(PRIO_PROCESS, target, n) == -1) {
            // 失敗常見原因：權限不足（嘗試提高優先權時）
            fprintf(stderr, "setpriority(pid=%d, nice=%d) -> %s\n",
                    (int)target, n, strerror(errno));
        } else {
            printf("setpriority(pid=%d, nice=%d) -> OK\n", (int)target, n);
        }

        // 驗證
        int now;
        if (get_nice_checked(PRIO_PROCESS, target, &now) == -1) {
            perror("getpriority");
        } else {
            printf("  verify getpriority -> %d\n", now);
        }

        // 稍微停一下，方便觀察
        usleep(100 * 1000); // 100 ms
    }

    // 最後再顯示一次
    if (get_nice_checked(PRIO_PROCESS, target, &cur) == 0)
        printf("[info] final nice = %d\n", cur);

    return 0;
}

