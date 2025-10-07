#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid;

    printf("Before fork\n");

    pid = fork();   // 建立 child process

    if (pid < 0) {
        // fork 失敗
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // 這裡是 child process
        printf("This is the child process. PID = %d, Parent PID = %d\n", getpid(), getppid());
    } else {
        // 這裡是 parent process
        printf("This is the parent process. PID = %d, Child PID = %d\n", getpid(), pid);
    }

    return 0;
}

