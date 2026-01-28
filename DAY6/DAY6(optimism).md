# 进程与进程管理（优化版）

## 1. 进程标识符 (PID)
- **类型**：`pid_t`（通常为有符号32位整数）
- **特性**：
  - PID范围：0-32767（可配置调整）
  - 顺序分配，但可复用（非文件描述符的最小可用原则）
  - 特殊PID：0（调度进程/swapper）、1（init/systemd进程）
- **相关函数**：
  ```c
  pid_t getpid(void);    // 获取当前进程PID
  pid_t getppid(void);   // 获取父进程PID
  ```

## 2. 进程创建：fork() 深度解析

### fork() 工作机制
- **内核中断机制**：触发中断 → 内核创建子进程PCB → 复制父进程映像
- **关键词**：Copy-on-Write (COW) - 写时复制，延迟物理内存复制

### 父子进程差异
| 特性 | 父进程 | 子进程 |
|------|--------|--------|
| fork()返回值 | 子进程PID | 0 |
| PID | 保持不变 | 新分配 |
| PPID | 保持不变 | 父进程PID |
| 未决信号 | 不继承 | 初始为空 |
| 文件锁 | 不继承 | 不继承 |
| 资源统计 | 累计 | 清零 |
| CPU时间 | 累计 | 从0开始 |

### 关键注意事项
1. **缓冲区刷新**：
   ```c
   fflush(NULL);  // fork()前刷新所有流
   pid_t pid = fork();
   ```
   - **原因**：缓冲数据会被复制到子进程，导致重复输出
   - **缓冲类型**：
     - 终端：行缓冲（`\n`自动刷新）
     - 文件：全缓冲（需手动刷新）
     - stderr：通常无缓冲

2. **vfork() vs fork()**
   - `vfork()`：创建后**立即**执行exec，子进程**共享**父进程地址空间
   - `fork()`：使用COW机制，适合需要独立内存的场景

## 3. 进程终止与资源回收

### 进程状态
```
正常终止 → 僵尸进程(Z) → wait() → 完全回收
   ↑          ↑
父进程终止   父进程未回收
   ↓
孤儿进程 → 被init(1)接管 → init回收
```

### 回收函数族
```c
#include <sys/wait.h>

// 基础回收
pid_t wait(int *status);  // 阻塞等待任意子进程

// 精确控制
pid_t waitpid(pid_t pid, int *status, int options);
/*
pid参数：
  >0: 指定子进程PID
  -1: 任意子进程
  0: 同组子进程
  <-1: 进程组ID为|pid|
options：
  WNOHANG: 非阻塞
  WUNTRACED: 包括停止的子进程
  WCONTINUED: 包括继续运行的子进程
*/

// 扩展功能
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
```

### 状态码解析
```c
if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);  // 正常退出码
} else if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);  // 终止信号
    WCOREDUMP(status);  // 是否产生core dump
} else if (WIFSTOPPED(status)) {
    int sig = WSTOPSIG(status);  // 停止信号
} else if (WIFCONTINUED(status)) {
    // 子进程继续执行
}
```

## 4. 进程映像替换：exec函数族

### 函数分类
```c
// l: 参数列表(List)，v: 参数向量(Vector)
// p: 使用PATH搜索，e: 环境变量(Environment)

execl("/bin/ls", "ls", "-l", NULL);      // 列表形式，完整路径
execlp("ls", "ls", "-l", NULL);          // 列表形式，PATH搜索
execle("/bin/ls", "ls", "-l", NULL, env);// 指定环境变量

execv("/bin/ls", args);                  // 向量形式，完整路径
execvp("ls", args);                      // 向量形式，PATH搜索
execvpe("ls", args, env);                // 向量形式，指定环境变量
```

### Shell工作原理模型
```c
// shell执行命令的简化实现
void shell_execute(const char *cmd) {
    pid_t pid = fork();
    if (pid == 0) {  // 子进程
        // 解析命令为argv
        execvp(argv[0], argv);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  // 父进程(shell)
        waitpid(pid, NULL, 0);
    }
}
```

### 关键特性
1. **文件描述符继承**：子进程继承父进程的打开文件表（包括stdin/stdout/stderr）
2. **信号处理重置**：exec后大部分信号恢复默认处理
3. **进程属性保留**：PID、PPID、优先级、资源限制等保持不变

## 5. 用户与组权限模型

### 三重身份机制
| 标识符类型 | 含义 | 作用 |
|-----------|------|------|
| Real UID/GID | 实际身份 | 登录用户/组 |
| Effective UID/GID | 有效身份 | 权限检查依据 |
| Saved UID/GID | 保存身份 | 权限切换的备份 |

### 权限相关函数
```c
// 获取身份
uid_t getuid(void);    // RUID
uid_t geteuid(void);   // EUID
gid_t getgid(void);    // RGID
gid_t getegid(void);   // EGID

// 设置身份
int setuid(uid_t uid);     // 同时设置RUID和EUID(有特权时)
int seteuid(uid_t uid);    // 仅设置EUID
int setreuid(uid_t ruid, uid_t euid);  // 分别设置，-1保持

// 组操作类似...
```

### SUID/SGID机制
```bash
# SUID: 执行时以文件所有者身份运行
chmod u+s /usr/bin/passwd  # -rwsr-xr-x

# SGID: 
#   文件：以文件所属组身份运行
#   目录：新建文件继承目录组
chmod g+s /shared/project  # drwxr-sr-x
```

### mysu实现原理
```c
// 简化的mysu核心逻辑
int main(int argc, char *argv[]) {
    // 1. 检查参数，获取目标用户信息
    struct passwd *pw = getpwnam(target_user);
    
    // 2. 权限检查（需要SUID位或root权限）
    if (geteuid() != 0) {
        fprintf(stderr, "Permission denied\n");
        exit(EXIT_FAILURE);
    }
    
    // 3. 设置目标身份
    setgid(pw->pw_gid);      // 设置真实和有效GID
    setuid(pw->pw_uid);      // 设置真实和有效UID
    
    // 4. 执行shell
    char *shell = pw->pw_shell ?: "/bin/sh";
    execl(shell, shell, NULL);
    
    // execl失败时
    perror("execl");
    return EXIT_FAILURE;
}
```

## 6. 解释器文件机制
```
#!解释器路径 [可选参数]
脚本内容...
```

**执行流程**：
1. 内核识别`#!`开头的文件
2. 加载解释器到内存
3. 将脚本文件作为参数传递给解释器
4. 解释器执行脚本内容

## 7. system() 函数
```c
int system(const char *command);
```
**等价实现**：
```c
int my_system(const char *cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);  // exec失败
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return status;
    }
    return -1;  // fork失败
}
```

**注意**：system()会创建shell进程，可能存在安全风险（命令注入）。

## 8. 进程会计
```c
#include <sys/acct.h>
int acct(const char *filename);  // filename为NULL停止记账
```
- 记录：命令名、用户、终端、起止时间、CPU时间等
- 需要root权限启用

## 9. 进程时间统计
```c
#include <sys/times.h>
clock_t times(struct tms *buf);

struct tms {
    clock_t tms_utime;   // 用户CPU时间
    clock_t tms_stime;   // 系统CPU时间
    clock_t tms_cutime;  // 子进程用户时间
    clock_t tms_cstime;  // 子进程系统时间
};
```

## 10. 守护进程设计与实现

### 会话与进程组概念
| 概念 | 描述 | 关系 |
|------|------|------|
| 会话(Session) | 终端关联的进程集合 | 包含1个或多个进程组 |
| 进程组(Process Group) | 相关进程的集合 | 属于1个会话 |
| 控制终端 | 会话的输入输出设备 | 与会话关联 |

**关键函数**：
```c
pid_t setsid(void);      // 创建新会话，成为会话首进程
pid_t getpgrp(void);     // 获取进程组ID
int setpgid(pid_t pid, pid_t pgid);  // 设置进程组
```

### 守护进程创建标准步骤
```c
int daemonize(void) {
    // 1. 第一次fork，脱离父进程
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0);  // 父进程退出
    
    // 2. 创建新会话，脱离终端
    if (setsid() < 0) return -1;
    
    // 3. 第二次fork，防止重新获取终端
    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) exit(0);
    
    // 4. 关闭文件描述符
    for (int fd = 0; fd < sysconf(_SC_OPEN_MAX); fd++)
        close(fd);
    
    // 5. 重定向标准流到/dev/null
    int nullfd = open("/dev/null", O_RDWR);
    dup2(nullfd, STDIN_FILENO);
    dup2(nullfd, STDOUT_FILENO);
    dup2(nullfd, STDERR_FILENO);
    if (nullfd > STDERR_FILENO) close(nullfd);
    
    // 6. 设置umask
    umask(0);
    
    // 7. 更改工作目录
    chdir("/");
    
    // 8. 设置信号处理
    signal(SIGHUP, SIG_IGN);
    
    return 0;
}
```

## 11. 系统日志记录

### syslog服务接口
```c
#include <syslog.h>

// 打开连接
void openlog(const char *ident, int option, int facility);
/*
option:
  LOG_PID     - 包含PID
  LOG_CONS    - 无法记录时输出到控制台
  LOG_NDELAY  - 立即连接
  LOG_ODELAY  - 延迟连接（默认）
  
facility:
  LOG_USER    - 普通用户程序（默认）
  LOG_DAEMON  - 守护进程
  LOG_AUTH    - 安全/认证
  LOG_LOCAL0-7- 本地使用
*/

// 记录日志
void syslog(int priority, const char *format, ...);
/*
priority = facility | level
level:
  LOG_EMERG   0 - 系统不可用
  LOG_ALERT   1 - 需要立即行动
  LOG_CRIT    2 - 严重错误
  LOG_ERR     3 - 错误
  LOG_WARNING 4 - 警告
  LOG_NOTICE  5 - 正常但重要
  LOG_INFO    6 - 信息
  LOG_DEBUG   7 - 调试
*/

// 关闭连接
void closelog(void);
```

### 使用示例
```c
int main() {
    openlog("mydaemon", LOG_PID | LOG_CONS, LOG_DAEMON);
    
    syslog(LOG_INFO, "守护进程启动，PID=%d", getpid());
    
    // 工作代码...
    
    if (error_occurred) {
        syslog(LOG_ERR, "操作失败: %m");  // %m自动展开strerror(errno)
    }
    
    closelog();
    return 0;
}
```

### 日志查看位置
```bash
# 传统syslog
sudo tail -f /var/log/syslog        # Ubuntu/Debian
sudo tail -f /var/log/messages      # CentOS/RHEL

# systemd journal
sudo journalctl -f                  # 实时查看
sudo journalctl _COMM=mydaemon      # 按程序名过滤
sudo journalctl -u service_name     # 按服务单元
```

## 实践要点总结

1. **fork-exec模型**：理解COW机制，正确处理缓冲区
2. **资源回收**：避免僵尸进程，合理使用wait/waitpid
3. **权限安全**：最小权限原则，谨慎使用SUID
4. **守护进程**：完整遵循创建步骤，正确处理信号
5. **错误处理**：检查所有系统调用返回值，使用perror或syslog记录
6. **可移植性**：注意不同系统对进程特性的支持差异

这种优化后的笔记结构更清晰，加入了对比表格、代码示例和实际应用场景，便于理解和记忆。