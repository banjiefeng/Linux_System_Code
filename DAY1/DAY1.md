# DAY1: 标准 I/O（stdio）

## 核心概念
- **I/O 基础**：I/O（Input & Output）是程序交互的基础
- **优先使用 stdio**：标准库 I/O 相比系统调用 (sysio) 能合并请求、利用缓冲，性能更优
- **FILE 对象管理**：`fopen()` 返回的 `FILE*` 由标准库管理，`fclose()` 负责释放和清理资源

## 常用函数清单

### 文件打开/关闭
```c
FILE *fopen(const char *pathname, const char *mode);  // man 3 fopen
int fclose(FILE *stream);                              // man 3 fclose
```

### 字符和行操作
```c
int fgetc(FILE *stream);    // 返回 int（检查 EOF = -1）
int fputc(int c, FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputs(const char *s, FILE *stream);
int putchar(int c);         // putc() 的另一种形式
```

### 块读写
```c
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
```

### 格式化 I/O
```c
// 输出
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

// 输入
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
```

## 文件定位与偏移
```c
int fseek(FILE *stream, long offset, int whence);      // 有缺陷，仅限 < 2GB 文件
long ftell(FILE *stream);
int fseeko(FILE *stream, off_t offset, int whence);    // 支持大文件
off_t ftello(FILE *stream);
void rewind(FILE *stream);                              // 回到文件首
```

## 缓冲与刷新

### 三种缓冲模式
1. **无缓冲**：立即输出（如 stderr）
2. **行缓冲**：换行时刷新（如 stdout）
3. **全缓冲**：缓冲满时刷新（如普通文件）

### 刷新方法
```c
int fflush(FILE *stream);      // 强制刷新流（或全缓冲）
int setvbuf(FILE *stream, char *buf, int mode, size_t size);  // 设置缓冲策略
```

## 常见错误处理

### errno 和错误消息
```c
#include <errno.h>
#include <string.h>

FILE *fp = fopen("file.txt", "r");
if (!fp) {
    perror("fopen");           // 打印错误信息
    printf("%s\n", strerror(errno));  // 或使用 strerror()
}
```

## getline 使用示例

**重要**：必须初始化 `line = NULL; len = 0;`，避免段错误

```c
#include <stdio.h>
#include <stdlib.h>

char *line = NULL;
size_t len = 0;
ssize_t nread;

while ((nread = getline(&line, &len, stdin)) != -1) {
    printf("Read %zd chars: %s", nread, line);
}

free(line);  // 释放 getline 分配的内存
```

**返回值**：
- `>= 0`：实际读取的字符数（包含换行符）
- `-1`：EOF 或错误

## 临时文件处理

### 推荐方案
```c
// 方案 1：mkstemp（最安全）
char template[] = "/tmp/myapp_XXXXXX";
int fd = mkstemp(template);
FILE *fp = fdopen(fd, "w+");
// 使用 fp...
fclose(fp);
unlink(template);  // 删除文件

// 方案 2：tmpfile（自动删除）
FILE *fp = tmpfile();  // 返回临时文件指针
// 使用 fp...
fclose(fp);  // 文件自动删除
```

### 避免使用
- `tmpnam()`（不安全，存在并发冲突风险）：两个并发进程可能生成相同文件名

## 字符串转换

### atoi 和相关函数
```c
int atoi(const char *str);        // "123" -> 123
long atol(const char *str);
long long atoll(const char *str);
```

## 系统限制

### 查看当前限制
```bash
ulimit -a              # 查看所有限制
ulimit -n              # 查看最大打开文件数
```

## 权限与掩码

### umask 工作原理
```c
// 新建文件的权限 = 请求权限 & ~umask
// 例如：0666 & ~0022 = 0644
umask(0022);           // 设置新掩码
```

## 关键总结

| 概念               | 说明                                                     |
| ------------------ | -------------------------------------------------------- |
| **stdio vs sysio** | 使用 stdio 获得更好的缓冲和性能                          |
| **返回值检查**     | 每个 I/O 函数都有特定的错误指示（如 `fgetc` 返回 `EOF`） |
| **资源释放**       | `fopen` 必须配对 `fclose`；`getline` 分配的内存需 `free` |
| **临时文件**       | 优先 `mkstemp()` 或 `tmpfile()`，避免 `tmpnam()`         |
| **大文件支持**     | 用 `fseeko()/ftello()` 而非 `fseek()/ftell()`            |