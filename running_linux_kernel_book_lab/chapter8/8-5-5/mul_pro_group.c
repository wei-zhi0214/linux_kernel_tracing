#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

/**
 * 显示进程组信息
 */
void show_process_group_info(pid_t pgid) {
    printf("进程组 %d 信息:\n", pgid);
    
    // 显示进程组中所有进程的优先级
    // 注意：这里简化处理，实际应用中需要遍历/proc或使用其他方法
    int current_prio = getpriority(PRIO_PGRP, pgid);
    if (current_prio != -1 || errno == 0) {
        printf("  进程组优先级: %d\n", current_prio);
    } else {
        printf("  无法获取进程组优先级: %s\n", strerror(errno));
    }
}

/**
 * 工作进程函数（进程组成员）
 */
void group_worker(int worker_id, pid_t group_leader) {
    pid_t pid = getpid();
    
    printf("组工作进程 %d 启动 (PID: %d)\n", worker_id, pid);
    
    // 加入指定的进程组
    if (setpgid(0, group_leader) == 0) {
        printf("组工作进程 %d 成功加入进程组 %d\n", worker_id, group_leader);
    } else {
        printf("组工作进程 %d 加入进程组失败: %s\n", worker_id, strerror(errno));
    }
    
    // 显示当前进程信息
    printf("组工作进程 %d 当前优先级: %d\n", worker_id, getpriority(PRIO_PROCESS, 0));
    printf("组工作进程 %d 所属进程组: %d\n", worker_id, getpgid(0));
    
    // 模拟工作
    for (int i = 0; i < 10; i++) {
        printf("组工作进程 %d 工作中... (%d/10)\n", worker_id, i + 1);
        sleep(1);
    }
    
    printf("组工作进程 %d 完成\n", worker_id);
    exit(0);
}

/**
 * 演示进程组优先级设置
 */
int demo_process_group_priority() {
    pid_t group_leader_pid;
    pid_t workers[2];
    pid_t group_id;
    
    printf("=== 进程组优先级设置演示 ===\n");
    
    // 创建进程组领导进程
    group_leader_pid = fork();
    if (group_leader_pid == 0) {
        // 进程组领导进程
        printf("进程组领导进程启动 (PID: %d)\n", getpid());
        
        // 创建自己的进程组
        if (setpgid(0, 0) == 0) {
            group_id = getpid();
            printf("成功创建进程组: %d\n", group_id);
        } else {
            printf("创建进程组失败: %s\n", strerror(errno));
            exit(1);
        }
        
        // 显示初始优先级
        printf("进程组领导进程初始优先级: %d\n", getpriority(PRIO_PROCESS, 0));
        
        // 创建组内工作进程
        for (int i = 0; i < 2; i++) {
            workers[i] = fork();
            if (workers[i] == 0) {
                group_worker(i + 1, group_id);
            } else if (workers[i] > 0) {
                printf("创建组工作进程 %d: PID=%d\n", i + 1, workers[i]);
            }
        }
        
        // 等待子进程启动
        sleep(1);
        
        // 设置整个进程组的优先级
        printf("\n设置进程组 %d 的优先级:\n", group_id);
        show_process_group_info(group_id);
        
        // 尝试设置进程组优先级
        printf("尝试将进程组 %d 的优先级设置为 5:\n", group_id);
        int result = setpriority(PRIO_PGRP, group_id, 5);
        if (result == 0) {
            printf("✓ 成功设置进程组优先级\n");
        } else {
            printf("✗ 设置进程组优先级失败: %s\n", strerror(errno));
            if (errno == EACCES) {
                printf("  需要适当权限来设置优先级\n");
            }
        }
        
        // 验证设置结果
        show_process_group_info(group_id);
        
        // 等待组内工作进程完成
        for (int i = 0; i < 2; i++) {
            int status;
            waitpid(workers[i], &status, 0);
            printf("组工作进程 %d 已完成\n", i + 1);
        }
        
        printf("进程组领导进程结束\n");
        exit(0);
    } else if (group_leader_pid > 0) {
        printf("主进程创建进程组领导进程: %d\n", group_leader_pid);
        
        // 等待进程组领导进程完成
        int status;
        waitpid(group_leader_pid, &status, 0);
        printf("进程组领导进程已结束\n");
    } else {
        perror("创建进程组领导进程失败");
        return -1;
    }
    
    return 0;
}

int main() {
    return demo_process_group_priority();
}

