#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

/**
 * 显示用户优先级信息
 */
void show_user_priority(uid_t uid) {
    struct passwd *pwd = getpwuid(uid);
    if (pwd) {
        printf("用户 %s (UID: %d) 信息:\n", pwd->pw_name, uid);
    } else {
        printf("用户 (UID: %d) 信息:\n", uid);
    }
    
    // 获取用户进程的优先级（注意：这会返回其中一个进程的优先级）
    errno = 0;
    int prio = getpriority(PRIO_USER, uid);
    if (prio != -1 || errno == 0) {
        printf("  用户进程优先级: %d\n", prio);
    } else {
        printf("  无法获取用户优先级: %s\n", strerror(errno));
        if (errno == ESRCH) {
            printf("  该用户可能没有运行中的进程\n");
        }
    }
}

/**
 * 演示用户优先级管理
 */
int demo_user_priority_management() {
    uid_t current_uid = getuid();
    uid_t effective_uid = geteuid();
    
    printf("=== 用户优先级管理演示 ===\n");
    
    // 显示当前用户信息
    printf("1. 当前用户信息:\n");
    printf("   真实用户ID: %d\n", current_uid);
    printf("   有效用户ID: %d\n", effective_uid);
    
    show_user_priority(current_uid);
    
    // 检查权限
    printf("\n2. 权限检查:\n");
    if (current_uid == 0 || effective_uid == 0) {
        printf("   ✓ 当前具有root权限\n");
    } else {
        printf("   ✗ 当前没有root权限\n");
        printf("   设置其他用户优先级需要root权限\n");
    }
    
    // 尝试设置当前用户优先级
    printf("\n3. 设置当前用户优先级:\n");
    int original_prio = getpriority(PRIO_USER, current_uid);
    if (original_prio == -1 && errno != 0) {
        original_prio = 0;  // 默认值
    }
    
    printf("   当前用户优先级: %d\n", original_prio);
    printf("   尝试设置为: %d\n", original_prio + 2);
    
    int result = setpriority(PRIO_USER, current_uid, original_prio + 2);
    if (result == 0) {
        printf("   ✓ 成功设置用户优先级\n");
        
        // 验证设置结果
        int new_prio = getpriority(PRIO_USER, current_uid);
        printf("   新用户优先级: %d\n", new_prio);
    } else {
        printf("   ✗ 设置用户优先级失败: %s\n", strerror(errno));
        if (errno == EACCES) {
            printf("   需要root权限来设置用户优先级\n");
        } else if (errno == EPERM) {
            printf("   权限不足\n");
        }
    }
    
    // 显示系统中其他用户（如果有权限）
    printf("\n4. 系统用户优先级信息:\n");
    struct passwd *pwd;
    int user_count = 0;
    
    // 重置密码文件指针
    setpwent();
    
    while ((pwd = getpwent()) != NULL && user_count < 5) {
        // 只显示一些系统用户
        if (pwd->pw_uid < 1000) {  // 系统用户通常UID较小
            show_user_priority(pwd->pw_uid);
            user_count++;
        }
    }
    
    endpwent();
    
    // 恢复原始优先级
    printf("\n5. 恢复原始优先级:\n");
    result = setpriority(PRIO_USER, current_uid, original_prio);
    if (result == 0) {
        printf("   ✓ 成功恢复原始优先级\n");
    } else {
        printf("   ✗ 恢复原始优先级失败: %s\n", strerror(errno));
    }
    
    return 0;
}

int main() {
    return demo_user_priority_management();
}

