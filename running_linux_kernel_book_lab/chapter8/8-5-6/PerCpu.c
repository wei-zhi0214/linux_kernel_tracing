#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 20

// 文件描述符
int fd = -1;

// 多线程调用函数
void* pthread_fx(void* args)
{
    int ret = ioctl(fd, 1, 0);
    if (ret == -1) {
        perror("ioctl");
    }
    return NULL;
}

int main(void)
{
    int ret = 0;

    // 打开设备
    fd = open("/dev/hellodr", O_RDWR);
    if (fd < 0) {
        perror("open /dev/hellodr");
        return 1;
    }

    // 建立多個 thread
    pthread_t tids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i) {
        ret = pthread_create(&tids[i], NULL, pthread_fx, NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        }
    }

    // 回收 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(tids[i], NULL);
    }

    // 關閉設備
    close(fd);
    return 0;
}

