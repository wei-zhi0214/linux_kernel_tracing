#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/klog.h>   // glibc 提供的宣告
static volatile sig_atomic_t g_stop = 0;
static const char *LOGFILE = "/var/log/oopswatchd.log";

/* ---- utilities ---- */
static void on_signal(int sig){ g_stop = 1; }

static void log_file(const char *fmt, ...) {
    FILE *fp = fopen(LOGFILE, "a");
    if (!fp) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fputc('\n', fp);
    fclose(fp);
}

static int is_oops_line(const char *s) {
    return (strstr(s, "Oops:") ||
            strstr(s, "BUG:") ||
            strstr(s, "kernel BUG at") ||
            strstr(s, "WARNING:") ||
            strstr(s, "Kernel panic") ||
            strstr(s, "panic"));
}

/* ---- daemonize ---- */
static void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);          // parent exits

    if (setsid() < 0) exit(1);     // become session leader

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);          // first child exits; second child is real daemon

    umask(027);
    chdir("/");

    // close inherited fds
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; --fd) close(fd);

    // reopen stdio to /dev/null
    int fd0 = open("/dev/null", O_RDWR);
    dup2(fd0, STDIN_FILENO);
    dup2(fd0, STDOUT_FILENO);
    dup2(fd0, STDERR_FILENO);
}

/* ---- /dev/kmsg reader (preferred) ---- */
static int run_kmsg_loop(void) {
    int fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
    if (fd < 0) return -1;

    char buf[4096];
    while (!g_stop) {
        // 每 5 秒輪詢
        struct timespec ts = { .tv_sec = 5, .tv_nsec = 0 };
        nanosleep(&ts, NULL);

        for (;;) {
            ssize_t n = read(fd, buf, sizeof(buf)-1);
            if (n < 0) {
                if (errno == EAGAIN) break;   // 沒新訊息
                if (errno == EINTR) continue;
                // 其他錯誤：重新開啟
                close(fd);
                fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
                if (fd < 0) return -1;
                break;
            }
            buf[n] = '\0';
            // /dev/kmsg 格式："<pri>,seq,ts,flags;message"
            char *msg = strchr(buf, ';');
            if (!msg) msg = buf; else msg++;

            if (is_oops_line(msg)) {
                openlog("oopswatchd", LOG_PID, LOG_KERN);
                syslog(LOG_ALERT, "Kernel Oops detected: %s", msg);
                closelog();

                log_file("ALERT: %s", msg);
            }
        }
    }
    close(fd);
    return 0;
}

/* ---- klogctl() fallback ---- */
static int run_klogctl_loop(void) {
    // 讀 ring buffer 大小
    int size = klogctl(10 /*SYSLOG_ACTION_SIZE_BUFFER*/, NULL, 0);
    if (size <= 0) size = 1<<20; // 1MB 預估
    char *buf = malloc(size + 1);
    if (!buf) return -1;

    // 記錄上次處理到的結尾，用簡單序列化避免重複（粗略方法）
    size_t last_len = 0;

    while (!g_stop) {
        struct timespec ts = { .tv_sec = 5, .tv_nsec = 0 };
        nanosleep(&ts, NULL);

        int n = klogctl(3 /*SYSLOG_ACTION_READ_ALL*/, buf, size);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(buf); return -1;
        }
        buf[n] = '\0';

        // 只處理新增的那部分
        if ((size_t)n > last_len) {
            char *p = buf + last_len;
            char *line = p;
            for (; *p; ++p) {
                if (*p == '\n') {
                    *p = '\0';
                    if (is_oops_line(line)) {
                        openlog("oopswatchd", LOG_PID, LOG_KERN);
                        syslog(LOG_ALERT, "Kernel Oops detected: %s", line);
                        closelog();
                        log_file("ALERT: %s", line);
                    }
                    line = p + 1;
                }
            }
            last_len = n;
            // 若 ring buffer 被覆蓋導致 last_len > n，下次從 0 開始
            if ((int)last_len > n) last_len = 0;
        }
    }
    free(buf);
    return 0;
}

int main(void) {
    signal(SIGTERM, on_signal);
    signal(SIGINT,  on_signal);

    daemonize();

    // 建立自己的 log 檔
    log_file("oopswatchd started.");

    // 先試 /dev/kmsg，失敗再退回 klogctl
    if (run_kmsg_loop() < 0) {
        log_file("/dev/kmsg not available, fallback to klogctl()");
        run_klogctl_loop();
    }

    log_file("oopswatchd stopped.");
    return 0;
}

