# sigaction() 使用简介

## 一、基础概念

**sigaction()** 是 Linux/Unix 中设置信号处理程序的函数，用于替代传统的 `signal()` 函数，提供更强大的控制能力。

## 二、函数原型

```c
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

**参数说明：**
- `signum`：要设置的信号编号（如 SIGINT、SIGTERM）
- `act`：新的信号处理配置
- `oldact`：保存原来的配置（可为NULL）

## 三、核心结构体

```c
struct sigaction {
    void (*sa_handler)(int);                    // 传统处理函数
    void (*sa_sigaction)(int, siginfo_t *, void *); // 扩展处理函数
    sigset_t sa_mask;                           // 处理期间阻塞的信号集
    int sa_flags;                               // 控制标志
};
```

## 四、常用配置模式

### 1. **基础模式**（最常用）
```c
struct sigaction sa;
sa.sa_handler = your_handler;    // 设置处理函数
sigemptyset(&sa.sa_mask);        // 清空阻塞集
sa.sa_flags = SA_RESTART;        // 系统调用自动重启
sigaction(SIGINT, &sa, NULL);    // 安装处理函数
```

### 2. **扩展信息模式**（获取发送者信息）
```c
struct sigaction sa;
sa.sa_sigaction = your_sigaction;  // 使用扩展函数
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_SIGINFO | SA_RESTART;  // 启用扩展信息
sigaction(SIGINT, &sa, NULL);
```

## 五、重要标志（sa_flags）

| 标志 | 说明 | 常用场景 |
|------|------|----------|
| **SA_RESTART** | 被信号中断的系统调用自动重启 | 网络服务器、需要稳定的程序 |
| **SA_SIGINFO** | 使用 sa_sigaction 获取详细信息 | 需要知道谁发送的信号 |
| **SA_NOCLDSTOP** | 子进程停止时不发送 SIGCHLD | 只关心子进程终止 |
| **SA_NOCLDWAIT** | 子进程终止不产生僵尸进程 | 自动清理子进程 |
| **SA_RESETHAND** | 处理一次后恢复默认 | 单次信号捕获 |

## 六、处理函数类型

### 1. **传统函数**
```c
void handler(int sig) {
    // sig: 信号编号
}
```

### 2. **扩展函数**（需要 SA_SIGINFO 标志）
```c
void sigaction_handler(int sig, siginfo_t *info, void *context) {
    // sig: 信号编号
    // info->si_pid: 发送者进程ID
    // info->si_uid: 发送者用户ID
}
```

## 七、阻塞掩码（sa_mask）

**作用**：在信号处理函数执行期间，阻塞指定的其他信号，防止被打断。

```c
// 在 SIGINT 处理期间阻塞 SIGTERM
sigemptyset(&sa.sa_mask);
sigaddset(&sa.sa_mask, SIGTERM);  // 添加要阻塞的信号
```

## 八、完整示例

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t flag = 0;

void handler(int sig) {
    flag = sig;  // 只设置标志
}

int main() {
    struct sigaction sa;
    
    // 配置信号处理
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    // 设置多个信号
    sigaction(SIGINT, &sa, NULL);   // Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // kill 命令
    
    while(!flag) {
        printf("运行中...\n");
        sleep(2);
    }
    
    printf("收到信号 %d，准备退出\n", flag);
    return 0;
}
```

## 九、最佳实践

1. **处理函数要简单**：只设置标志，复杂逻辑在主循环中处理
2. **使用 SA_RESTART**：避免系统调用被意外中断
3. **合理设置阻塞掩码**：防止信号处理被打断
4. **使用原子变量**：信号处理函数与主程序通信

## 十、快速选择指南

- **普通程序**：基础模式 + SA_RESTART
- **服务程序**：扩展模式 + SA_SIGINFO + SA_RESTART
- **多进程程序**：根据需要使用 SA_NOCLD*
- **特殊需求**：按需组合标志

sigaction() 的核心是 **控制力**：可以精确控制信号处理的各个方面，比 signal() 更安全可靠。