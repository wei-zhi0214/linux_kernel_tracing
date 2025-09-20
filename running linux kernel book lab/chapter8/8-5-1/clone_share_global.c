#define _GNU_SOURCE
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/types.h>

static volatile int g = 0;      // 全域變數
static volatile int mode_thread = 0;

static pid_t gettid_sys(void){ return (pid_t)syscall(SYS_gettid); }

static int child_fn(void *arg) {
    (void)arg;
    printf("[child ] PID=%d TID=%d PPID=%d, initial g=%d\n",
           getpid(), gettid_sys(), getppid(), g);

    // 讓 parent 有時間先動作
    usleep(200*1000);

    // 子端修改 g
    g += 10;
    printf("[child ] after g+=10, g=%d (PID=%d,TID=%d,PPID=%d)\n",
           g, getpid(), gettid_sys(), getppid());

    // 再等一下看父是否已退出/被領養（proc 模式會看到 PPID 改變）
    usleep(300*1000);
    printf("[child ] final  g=%d, PPID=%d\n", g, getppid());
    return 0;
}

static void *alloc_stack(size_t sz){
    void *stk = mmap(NULL, sz, PROT_READ|PROT_WRITE,
                     MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    if (stk == MAP_FAILED) { perror("mmap stack"); exit(1); }
    return (char*)stk + sz; // 返回 stack top (down-growing)
}

int main(int argc, char **argv){
    if (argc >= 2 && strcmp(argv[1], "--thread")==0) mode_thread = 1;

    printf("Mode: %s\n", mode_thread ? "thread (CLONE_VM|CLONE_THREAD)" : "proc (separate addr space)");
    printf("[parent] PID=%d TID=%d, start g=%d\n", getpid(), gettid_sys(), g);

    size_t STK = 1<<20;
    void *child_stack = alloc_stack(STK);

    int flags;
    if (mode_thread) {
        flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD;
    } else {
        // 像 fork 的子行程（不共用位址空間）
        flags = SIGCHLD; // 沒有 CLONE_VM/CLONE_THREAD
    }

    int child_tid = clone(child_fn, child_stack, flags, NULL);
    if (child_tid == -1) { perror("clone"); exit(1); }

    // 父端先改 g
    g += 1;
    printf("[parent] after g+=1, g=%d (child_tid/ret=%d)\n", g, child_tid);

    if (!mode_thread) {
        // ---- proc 模式：父「先結束」，看子是否繼續並被領養 ----
        printf("[parent] exiting early in proc mode (child should continue; child will see new PPID)\n");
        _exit(0);  // 只結束父行程本身；子行程繼續、會被 reparent 給 PID 1
    } else {
        // ---- thread 模式：示範兩種父退出方式 ----
        // A) 若用 exit(0) 或從 main return -> 會呼叫 exit_group，整個行程與所有 threads 結束
        // printf("[parent] calling exit(0) in thread mode -> will kill all threads\n");
        // exit(0);

        // B) 僅結束父這個 thread，讓子 thread 繼續（示範共享 g）
        printf("[parent] thread mode: exiting ONLY this thread via SYS_exit; child thread keeps running\n");
        syscall(SYS_exit, 0); // 結束目前 thread，不是整個 process
    }

    return 0; // 理論上到不了
}

