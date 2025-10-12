#  Process Management – Chapter 8 Labs  
## **Lab: Per-CPU **

###  **Purpose**
How to use **PER-CPU variable** ,  

---

###  **Lab Requirement**

1. 寫一個簡單的核心模組,創建一個per-cpu變數,並且初始化該per-cpu變數,修改該per-cpu變數的值,然後輸出這些職

---
### Kernel Module
declare PER-CPU varible

```C
DEFINE_PER_CPU( long, gUsage ) = 0;
```
define ioctl to plus 1 to Per-CPU varible
```C
long DriverIOControl( struct file *pslFileStruct, unsigned int uiCmd, unsigned long ulArg )
{
    long *pUsage = NULL;
    /* printk( KERN_ALERT DEVICE_NAME ": pUsage = 0x%lx %lx %ld", (unsigned long) pUsage, (unsigned long) (&gUsage), (*pUsage) ); */
    preempt_disable();
    pUsage = this_cpu_ptr( (long *) (&gUsage) );
    (*pUsage)++;
    preempt_enable();
    return(0);
}
```
Note:
使用 this_cpu_ptr() 可以直接取得目前 CPU 對應的變數。
在進行修改前關閉搶佔 (preempt_disable())，以避免 CPU 切換導致誤更新。
---

### App
Create 20 threads, each thread call ioctl.
```C
    for (int i = 0; i < NUM_THREADS; ++i) {
        ret = pthread_create(&tids[i], NULL, pthread_fx, NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        }
    }
```
```C
    void* pthread_fx(void* args)
    {
        int ret = ioctl(fd, 1, 0);
        if (ret == -1) {
            perror("ioctl");
        }
        return NULL;
    }
```

###   **Execution Example**
```bash
insmod PerCpuDriver.ko
mknod /dev/hellodr c 231 0
./PerCpu
```
```
[   87.895242] hellodr hello open.
[   87.934597] hellodr pUsage = ffff88803ec2a298, *pUsage = 5
[   87.934837] hellodr pUsage = ffff88803ecaa298, *pUsage = 1
[   87.934906] hellodr pUsage = ffff88803ed2a298, *pUsage = 8
[   87.934972] hellodr pUsage = ffff88803edaa298, *pUsage = 6
[   87.935034] hellodr 20
```

### Result & Analysis
|   CPU ID  | Address (gUsage) | *pUsage | Description                  |
| :-------: | :--------------- | ------: | :--------------------------- |
|    CPU0   | ffff88803ec2a298 |       5 | Thread operations on CPU0    |
|    CPU1   | ffff88803ecaa298 |       1 | Thread operations on CPU1    |
|    CPU2   | ffff88803ed2a298 |       8 | Thread operations on CPU2    |
|    CPU3   | ffff88803edaa298 |       6 | Thread operations on CPU3    |
| **Total** | —                |  **20** | ✅ Matches total thread count |

Observation:
Four unique addresses correspond to four physical CPU cores.
Each Per-CPU variable maintains its own counter, and the sum equals the total number of ioctl invocations (20).