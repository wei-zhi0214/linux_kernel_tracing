// limiter.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

static int parse_bytes(const char *s, rlim_t *out) {
    // 支援 K/M/G 後綴，例如: 128M, 1G, 256K
    char *end = NULL;
    errno = 0;
    unsigned long long val = strtoull(s, &end, 10);
    if (errno != 0) return -1;
    rlim_t base = (rlim_t)val;
    if (*end == '\0') { *out = base; return 0; }
    if (*end == 'K' || *end == 'k') { *out = base * 1024ULL; return 0; }
    if (*end == 'M' || *end == 'm') { *out = base * 1024ULL * 1024ULL; return 0; }
    if (*end == 'G' || *end == 'g') { *out = base * 1024ULL * 1024ULL * 1024ULL; return 0; }
    return -1;
}

static void set_limit(int resource, rlim_t soft, rlim_t hard) {
    struct rlimit lim = { .rlim_cur = soft, .rlim_max = hard };
    if (setrlimit(resource, &lim) != 0) {
        perror("setrlimit");
        exit(1);
    }
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [--as <bytes|K|M|G>] [--stack <bytes|K|M|G>] "
        "[--cpu <seconds>] [--nofile <N>] -- <program> [args...]\n"
        "Examples:\n"
        "  %s --as 256M --cpu 2 -- ./memhog 512M\n"
        "  %s --nofile 64 -- ./openfiles 100\n",
        prog, prog, prog);
    exit(1);
}

int main(int argc, char **argv) {
    rlim_t as = 0, stack = 0, nofile = 0;
    rlim_t cpu = 0;
    int have_as=0, have_stack=0, have_cpu=0, have_nofile=0;

    int i = 1;
    for (; i < argc; ++i) {
        if (strcmp(argv[i], "--") == 0) { i++; break; }
        else if (strcmp(argv[i], "--as") == 0) {
            if (++i >= argc || parse_bytes(argv[i], &as) != 0) usage(argv[0]);
            have_as = 1;
        } else if (strcmp(argv[i], "--stack") == 0) {
            if (++i >= argc || parse_bytes(argv[i], &stack) != 0) usage(argv[0]);
            have_stack = 1;
        } else if (strcmp(argv[i], "--cpu") == 0) {
            if (++i >= argc) usage(argv[0]);
            char *end=NULL; errno=0;
            unsigned long long sec = strtoull(argv[i], &end, 10);
            if (errno || *end!='\0') usage(argv[0]);
            cpu = (rlim_t)sec; have_cpu = 1;
        } else if (strcmp(argv[i], "--nofile") == 0) {
            if (++i >= argc) usage(argv[0]);
            char *end=NULL; errno=0;
            unsigned long long n = strtoull(argv[i], &end, 10);
            if (errno || *end!='\0') usage(argv[0]);
            nofile = (rlim_t)n; have_nofile = 1;
        } else {
            usage(argv[0]);
        }
    }

    if (i >= argc) usage(argv[0]); // 必須有 -- <program> ...

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        // child: 設定限制
        if (have_as)     set_limit(RLIMIT_AS,     as,     as);
        if (have_stack)  set_limit(RLIMIT_STACK,  stack,  stack);
        if (have_cpu)    set_limit(RLIMIT_CPU,    cpu,    cpu);
        if (have_nofile) set_limit(RLIMIT_NOFILE, nofile, nofile);

        // 執行目標程式
        execvp(argv[i], &argv[i]);
        perror("execvp");
        _exit(127);
    } else {
        // parent: 等待子行程結束，回報原因（被 signal or 正常退出）
        int st;
        if (waitpid(pid, &st, 0) < 0) { perror("waitpid"); return 1; }
        if (WIFEXITED(st)) {
            fprintf(stderr, "[limiter] child exited with code %d\n", WEXITSTATUS(st));
            return WEXITSTATUS(st);
        } else if (WIFSIGNALED(st)) {
            int sig = WTERMSIG(st);
            fprintf(stderr, "[limiter] child killed by signal %d\n", sig);
            return 128 + sig;
        } else {
            fprintf(stderr, "[limiter] child ended unexpectedly\n");
            return 1;
        }
    }
}

