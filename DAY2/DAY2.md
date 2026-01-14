# 系统调用 IO

## 一、系统调用追踪

**strace** - 追踪系统调用
```bash
strace command  # 显示程序执行过程中的所有系统调用
```

---

## 二、文件描述符 vs 文件指针

| 概念 | 类型 | 说明 |
|------|------|------|
| **文件描述符 (fd)** | int | 进程文件打开表的下标，由内核维护 |
| **文件指针 (FILE\*)** | 结构体 | 标准 IO 库的抽象，包含缓冲区等信息 |

### 关键概念

- **文件打开表结构**：进程文件打开表 → 系统文件打开表 → inode 表
- **进程默认打开的 fd**：0(stdin)、1(stdout)、2(stderr)
- **fd 分配策略**：优先使用当前可用范围内最小的 fd
- **ulimit**：限制单个进程最多打开的文件数

### inode 查找过程

```
1. 根据文件名在目录文件中查找 inode 号
2. 将 inode 从磁盘读取到内存 inode 缓存
3. 在系统文件打开表中创建表项（关联 inode）
4. 在进程文件打开表中创建表项（指向系统表项）
5. 返回该表项的下标（文件描述符）
```

---

## 三、5 个核心系统调用

所有标准 IO (`fopen/fclose/read/write` 等) 都基于这 5 个系统调用实现：

| 系统调用 | 标准 IO | 功能 |
|---------|--------|------|
| `open()` | `fopen()` | 打开文件，返回 fd |
| `close()` | `fclose()` | 关闭文件 |
| `read()` | `fread()/getc()` | 读取数据 |
| `write()` | `fwrite()/putc()` | 写入数据 |
| `lseek()` | `fseek()` | 文件指针定位 |

### 标准 IO vs 系统调用 IO

```
标准 IO → 用户缓冲区 → 内核缓冲区 → 磁盘    (吞吐量大，有延迟)
系统调用 → 内核 → 磁盘                      (直接，速度快)
```

### 转换函数

```c
int fileno(FILE *stream);           // FILE* → fd
FILE *fdopen(int fd, const char *m); // fd → FILE*
```

### open() 的标志位映射

```c
fopen() 模式       → open() 标志
─────────────────────────────────
"r"               → O_RDONLY
"r+"              → O_RDWR
"w"               → O_WRONLY | O_CREAT | O_TRUNC
"w+"              → O_RDWR | O_CREAT | O_TRUNC
"a"               → O_WRONLY | O_CREAT | O_APPEND
"a+"              → O_RDWR | O_CREAT | O_APPEND
```

### open() 函数签名

```c
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```

- **flags 包含 O_CREAT 时必须指定 mode**（权限）
- **实际权限 = mode & ~umask**

#### 使用示例

```c
// 创建新文件，权限为 0600（用户可读可写）
int fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);

// 打开现有文件进行读写
int fd = open("file.txt", O_RDWR);

// 关闭文件
close(fd);
```

---

## 四、IO 效率对比

| 方式 | 缓冲 | 系统调用频率 | 吞吐量 | 延迟 |
|------|------|-----------|--------|------|
| 标准 IO | 有（用户缓冲） | 少 | ✅ 大 | ✅ 小 |
| 系统调用 IO | 无 | 多 | ❌ 小 | ❌ 大 |

**结论**：大批量数据用标准 IO，单次操作用系统调用 IO

---

## 五、文件共享与删除行的问题

### 场景：写一个程序删除文件的第 10 行

**核心技术点**：
- 使用 `lseek()` 定位文件位置
- 使用两个 fd 分别维护读、写位置（避免频繁 lseek）
- 将第 11 行之后的数据向前移动
- 使用 `ftruncate()` 截断文件

**相关函数**：
```c
int ftruncate(int fd, off_t length);  // 将文件截断到指定长度
int truncate(const char *path, off_t length);  // 文件名版本
```

---

## 六、原子操作与重定向

### 原子操作
不可分割的操作，解决竞争和冲突

### 文件重定向：dup 和 dup2

#### 1. 手动重定向（不推荐）
```c
close(1);  // 关闭标准输出
fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
// 此时 fd 会占据 1 号位置（最小可用）
puts("Hello");  // 输出到 file.txt
close(fd);
```
**缺点**：close 和 open 之间存在竞争窗口

#### 2. 使用 dup() 重定向
```c
fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
close(1);
dup(fd);  // 复制 fd 到最小可用 fd（即 1）
close(fd);
```
**缺点**：
- 需手动 close(1)，仍有竞争窗口
- 若 fd 本身为 1，逻辑复杂

#### 3. 使用 dup2() 重定向（推荐）✅
```c
fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
dup2(fd, 1);  // 原子操作：关闭 1 并复制 fd 到 1
close(fd);
```
**优点**：原子操作，无竞争窗口

---

## 七、数据同步函数

```c
void sync(void);                 // 同步所有文件系统的脏页到磁盘（全局）
int fsync(int fd);               // 同步指定 fd 的数据和元数据到磁盘
int fdatasync(int fd);           // 仅同步指定 fd 的数据到磁盘（不含元数据）
```

**性能**：`fdatasync > fsync >> sync`

---

## 八、高级接口

```c
int fcntl(int fd, int cmd, ...);   // 文件控制（锁、标志等）
int ioctl(int fd, int cmd, ...);   // 设备 IO 控制
```

**虚目录**：`/dev/fd/` - 进程的文件描述符映射目录
```bash
ls -l /proc/self/fd/   # 查看当前进程打开的所有 fd
```