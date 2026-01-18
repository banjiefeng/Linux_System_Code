# 进程环境 - 详细总结

## 一、main函数
```c
int main(int argc, char *argv[]);
```
- **argc**: 命令行参数个数（包括程序名本身）
- **argv**: 指向参数数组的指针，argv[0]通常是程序名

## 二、进程终止方式

### 正常终止（5种）
1. **从main函数返回**
   - 返回值通过 `echo $?` 查看
   - 一般约定：0表示成功，非0表示错误

2. **调用exit()**
   ```c
   void exit(int status);
   ```
   - 执行顺序：
     1. 调用所有`atexit()`注册的函数（先进后出）
     2. 执行标准I/O清理操作（刷新所有流）
     3. 调用`_exit()`或`_Exit()`结束进程

3. **调用_exit()或_Exit()**
   ```c
   void _exit(int status);
   void _Exit(int status);  // ISO C版本
   ```
   - 立即终止进程，**不执行**钩子函数和I/O清理

4. **最后一个线程从其启动例程返回**
   - 多线程环境中的正常终止方式

5. **最后一个线程调用pthread_exit()**
   - 多线程环境中的线程终止

### 异常终止（3种）
1. **调用abort()**
   ```c
   void abort(void);
   ```
   - 产生SIGABRT信号终止进程

2. **接到一个信号并终止**
   - 如：SIGSEGV（段错误）、SIGKILL（强制终止）

3. **最后一个线程对其取消请求被迫作出响应**
   - 线程被其他线程取消

## 三、终止处理函数（钩子函数）

### atexit()
```c
#include <stdlib.h>
int atexit(void (*function)(void));
```
- **功能**：注册进程正常终止时调用的函数
- **特点**：
  - 调用顺序：**先进后出**（LIFO）
  - 可注册多个函数（最少32个）
  - 仅限正常终止时调用

### 应用示例：资源自动清理
```c
void close_file1(void) { close(fd1); }
void close_file2(void) { close(fd2); }

int main() {
    fd1 = open("file1", O_RDONLY);
    if (fd1 < 0) {
        perror("open file1");
        exit(1);
    }
    atexit(close_file1);  // 注册关闭函数
    
    fd2 = open("file2", O_RDONLY);
    if (fd2 < 0) {
        perror("open file2");
        // 不需要手动关闭fd1，atexit会自动处理
        exit(1);
    }
    atexit(close_file2);
    
    // ... 程序逻辑
    
    return 0;  // 自动调用close_file2()，然后close_file1()
}
```

## 四、命令行参数解析

### getopt() - 短选项解析
```c
#include <unistd.h>
int getopt(int argc, char *const argv[], 
           const char *optstring);
extern char *optarg;  // 选项参数值
extern int optind;    // 下一个argv索引
extern int opterr;    // 错误输出标志
extern int optopt;    // 未知选项字符
```

**optstring格式**：
- `a`：无参数选项
- `a:`：必须带参数的选项
- `ab:c::`：组合说明
  - `a`：无参数
  - `b:`：必须有参数
  - `c::`：可选参数

**特殊处理**：
- `-`开头的参数被视为选项
- `--`表示选项结束，后续均为非选项参数
- 非选项参数使getopt返回-1

### getopt_long() - 长短选项解析
```c
#include <getopt.h>
int getopt_long(int argc, char *const argv[],
               const char *optstring,
               const struct option *longopts,
               int *longindex);
```

**option结构体**：
```c
struct option {
    const char *name;    // 长选项名
    int has_arg;         // 0无参，1必参，2选参
    int *flag;           // 返回方式控制
    int val;             // 返回值
};
```

## 五、环境变量

### 基本操作
```c
#include <stdlib.h>
extern char **environ;  // 全局环境变量数组

char *getenv(const char *name);      // 获取环境变量
int setenv(const char *name, const char *value, int overwrite);  // 设置/添加
int unsetenv(const char *name);      // 删除
int putenv(char *string);            // 设置（传统方法，不安全）
```

### 环境变量格式
```
KEY=VALUE
例如：PATH=/usr/bin:/bin，HOME=/home/user
```

## 六、C程序存储空间布局

**查看方法**：`pmap PID` 或 `cat /proc/PID/maps`

**典型布局**（从高地址到低地址）：
1. **内核空间**（用户进程不可访问）
2. **栈（Stack）**
   - 自动变量、函数调用信息
   - 向下增长
3. **堆（Heap）**
   - 动态内存分配（malloc/free）
   - 向上增长
4. **未初始化数据段（BSS）**
   - 未初始化的全局/静态变量
   - 程序启动时初始化为0
5. **已初始化数据段（Data）**
   - 已初始化的全局/静态变量
6. **代码段（Text）**
   - 机器指令，只读
   - 包含字符串常量

## 七、库管理

### 1. 静态库（.a）
- 链接时复制到可执行文件
- 优点：独立，不依赖运行时环境
- 缺点：体积大，更新需重新编译

### 2. 动态库（.so）
- 运行时加载
- 优点：共享，节省内存，易于更新
- 缺点：依赖库文件存在

### 3. 动态加载库（手工装载）
```c
#include <dlfcn.h>
void *dlopen(const char *filename, int flag);  // 打开库
void *dlsym(void *handle, const char *symbol); // 查找符号
char *dlerror(void);                           // 错误信息
int dlclose(void *handle);                     // 关闭库
```

**编译时需加链接选项**：`-ldl`

## 八、函数跳转

### goto的局限性
- 只能在当前函数内跳转
- 不能跨函数跳转

### 跨函数安全跳转
```c
#include <setjmp.h>
int setjmp(jmp_buf env);           // 设置跳转点
void longjmp(jmp_buf env, int val); // 跳转到跳转点
```

**使用示例**：
```c
jmp_buf env;

void func() {
    printf("准备跳转\n");
    longjmp(env, 1);  // 跳转到setjmp处
}

int main() {
    if (setjmp(env) == 0) {
        printf("首次设置跳转点\n");
        func();
    } else {
        printf("从longjmp跳转回来\n");
    }
    return 0;
}
```

## 九、资源限制与控制

### 资源限制结构体
```c
#include <sys/resource.h>
struct rlimit {
    rlim_t rlim_cur;  // 当前（软）限制
    rlim_t rlim_max;  // 最大（硬）限制
};
```

### 常用资源类型（RLIMIT_*）
- `RLIMIT_CORE`：core文件大小
- `RLIMIT_CPU`：CPU时间（秒）
- `RLIMIT_DATA`：数据段大小
- `RLIMIT_FSIZE`：文件大小
- `RLIMIT_NOFILE`：打开文件数
- `RLIMIT_STACK`：栈大小

### 操作函数
```c
int getrlimit(int resource, struct rlimit *rlim);
int setrlimit(int resource, const struct rlimit *rlim);
```

### 权限规则
| 用户类型 | 软限制 | 硬限制 |
|---------|--------|--------|
| 普通用户 | 可升高/降低 | 只能降低，不能升高 |
| root用户 | 可升高/降低 | 可升高/降低 |

**关系**：软限制 ≤ 硬限制

## 十、重要实践要点

1. **退出状态码**：
   - 0：成功
   - 1-125：用户定义的错误
   - 126：命令找到但不可执行
   - 127：命令未找到
   - 128+：信号终止（128+信号编号）

2. **钩子函数最佳实践**：
   - 用于资源清理（文件、内存、锁等）
   - 避免在钩子函数中调用exit()
   - 注意调用顺序（LIFO）

3. **环境变量安全**：
   - 优先使用setenv()而非putenv()
   - 注意内存管理，setenv()会复制字符串

4. **资源限制设置**：
   - 程序启动时检查/设置资源限制
   - 考虑安全问题，防止资源耗尽攻击

5. **错误处理**：
   - 检查所有系统调用的返回值
   - 使用perror()或strerror()输出错误信息