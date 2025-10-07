#include <sys/klog.h>
#include <stdio.h>
#include <unistd.h>
#define LOG_BUF_SIZE (1024 * 1024)
int main()
{
    char buf[LOG_BUF_SIZE] = {0};
    while(1)
    {
	    int byteCount = klogctl(2, buf, LOG_BUF_SIZE - 1);    /* 4 -- Read and clear all messages remaining in the ring buffer */
	    printf("Data read from kernel ring buffer: %s\n", buf);
	    sleep(1);
    }
    return 0;
}
