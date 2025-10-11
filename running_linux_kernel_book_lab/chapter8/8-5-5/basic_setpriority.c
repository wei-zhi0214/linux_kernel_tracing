#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

/**
 * 显示进程优先级信息
 */
void show_process_priority() {
    pid_t pid = getpid();
    uid_t uid = getuid();
    int prio;
    
    printf("=== 进程优先级信息 ===\n");
    printf("进程ID: %d\n", pid);
    printf("用户ID: %d\n", uid);
    
    // 获取当前进程优先级
    errno = 0;
    prio = getpriority(PRIO_PROCESS, 0);
    if (prio == -1 && errno != 0) {
        printf("获取进程优先级失败: %s\n", strerror(errno));
    } else {
        printf("当前进程优先级 (nice值): %d\n", prio);
    }
    
    printf("\n");
}

/**
 * 演示基础setpriority使用方法
 */
int demo_setpriority_basic() {
    int original_prio, new_prio;
    int result;
    
    printf("=== 基础setpriority使用示例 ===\n");
    
    // 显示原始优先级信息
    show_process_priority();
    original_prio = getpriority(PRIO_PROCESS, 0);
    printf("原始优先级: %d\n", original_prio);
    
    // 提高进程优先级（降低nice值）
    printf("1. 提高进程优先级 (nice值从 %d 降到 %d):\n", original_prio, original_prio - 5);
    result = setpriority(PRIO_PROCESS, 0, original_prio - 5);
    
    if (result == 0) {
        printf("   ✓ 成功提高优先级\n");
        
        // 验证设置结果
        new_prio = getpriority(PRIO_PROCESS, 0);
        printf("   新优先级: %d\n", new_prio);
        
        if (new_prio == original_prio - 5) {
            printf("   ✓ 优先级设置正确\n");
        } else {
            printf("   ✗ 优先级设置可能有问题\n");
        }
    } else {
        printf("   ✗ 提高优先级失败: %s\n", strerror(errno));
        if (errno == EACCES) {
            printf("   原因：需要root权限或CAP_SYS_NICE能力来提高优先级\n");
        } else if (errno == EINVAL) {
            printf("   原因：优先级值超出有效范围\n");
        } else if (errno == ESRCH) {
            printf("   原因：指定的进程不存在\n");
        }
    }
    
    // 降低进程优先级（提高nice值）
    printf("\n2. 降低进程优先级 (nice值从 %d 升到 %d):\n", new_prio, new_prio + 10);
    result = setpriority(PRIO_PROCESS, 0, new_prio + 10);
    
    if (result == 0) {
        printf("   ✓ 成功降低优先级\n");
        
        // 验证设置结果
        new_prio = getpriority(PRIO_PROCESS, 0);
        printf("   新优先级: %d\n", new_prio);
    } else {
        printf("   ✗ 降低优先级失败: %s\n", strerror(errno));
    }
    
    // 恢复原始优先级
    printf("\n3. 恢复原始优先级 (%d):\n", original_prio);
    result = setpriority(PRIO_PROCESS, 0, original_prio);
    
    if (result == 0) {
        printf("   ✓ 成功恢复原始优先级\n");
        new_prio = getpriority(PRIO_PROCESS, 0);
        printf("   最终优先级: %d\n", new_prio);
    } else {
        printf("   ✗ 恢复原始优先级失败: %s\n", strerror(errno));
    }
    
    return 0;
}

int main() {
    return demo_setpriority_basic();
}

