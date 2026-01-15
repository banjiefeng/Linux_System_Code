# 文件系统

**类 `ls` 的实现（如 `myls`）**

---

## 一、目录和文件

### 1、获取文件属性（stat 系列函数）

* **stat**

  * 通过**文件路径**获取文件属性
  * 若路径是符号链接，获取的是**目标文件**的属性

* **fstat**

  * 通过**文件描述符**获取文件属性

* **lstat**

  * 通过**文件路径**获取文件属性
  * 若路径是符号链接，获取的是**链接文件本身**的属性

📌 相关手册：

* `man 2 stat`（系统调用）
* `man 3 stat`（`struct stat` 结构体）
* `man 7 inode`

#### `struct stat` 中的重要成员 —— `st_mode`

* `st_mode` 是一个 **16 位位图**
* 同时描述：

  * **文件类型**
  * **文件权限**
* 文件类型判断宏（`man 7 inode`）：

  * `S_ISREG()`：普通文件
  * `S_ISDIR()`：目录
  * `S_ISLNK()`：符号链接
  * `S_ISCHR()` / `S_ISBLK()`：字符/块设备

```
位号        含义                     位数
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
0–2       其他用户权限 (o)          3 位 (r w x)
3–5       所属组权限 (g)            3 位 (r w x)
6–8       所有者权限 (u)            3 位 (r w x)
9–11      特殊权限位                3 位 (sticky / setgid / setuid)
12–15     文件类型                  4 位
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

### 2、获取文件访问权限

* 使用 `access()` 判断**当前进程**是否具有：

  * 读权限（`R_OK`）
  * 写权限（`W_OK`）
  * 执行权限（`X_OK`）

📌 区别说明：

* `stat`：查看**文件本身的权限**
* `access`：以**进程真实 UID/GID**判断是否“能访问”

---

### 3、umask（文件权限掩码）

* 作用：**防止新建文件或目录权限过松**
* `man 2 umask`
* `umask` 会影响：

  * `open`
  * `creat`
  * `mkdir`

```
最终权限 = 创建时指定权限 & ~umask
```

📌 示例：

```c
umask(022);
```

---

### 4、文件权限的更改 / 管理

* `chmod`：通过**路径名**修改权限
* `fchmod`：通过**文件描述符**修改权限

---

### 5、粘住位（Sticky Bit，t 位）

* 主要用于**目录**
* 常见于 `/tmp`

规则：

* ✅ 目录所有者：可删除目录中任意文件
* ✅ 文件所有者：可删除自己的文件
* ❌ 普通用户：不能删除其他用户的文件

  > 即使该文件对其可写

---

### 6、文件系统类型

* **FAT**

  * 无权限、无 inode 概念
* **UFS / ext4**

  * 支持 inode、权限、硬链接等

📌 `ls` 的实现依赖于 **inode + 权限模型**

---

### 7、硬链接与符号链接

#### 硬链接（Hard Link）

```bash
ln source destination
```

* 多个目录项指向**同一个 inode**
* inode 引用计数增加
* 不可跨文件系统
* 不能对目录创建（一般用户）

#### 符号链接（Soft Link）

```bash
ln -s source destination
```

* 有**独立 inode**
* inode 中保存的是**目标路径字符串**
* 可跨文件系统
* 目标删除后形成“悬空链接”

#### 相关系统调用

* `link`（man 2）：创建硬链接
* `unlink`：删除目录项（硬链接数 -1）
* `remove`（man 3）：删除文件或空目录
* `rename`（man 2）：重命名或移动（等价 `mv`）

---

### 8、utime

* `man 2 utime`
* 用于修改文件时间戳：

  * **atime**（最后访问时间）
  * **mtime**（最后修改时间）

---

### 9、目录的创建和销毁

* `mkdir`（man 2）：创建目录
* `rmdir`（man 2）：删除**空目录**

📌 删除非空目录需使用 `rm -r`（用户态命令）

---

### 10、更改当前工作目录

* `chdir`
* `fchdir`
* 等价于 shell 命令：`cd`

📌 特点：

* 修改的是**当前进程**的工作目录

* 可配合 `chroot` 使用（假根环境）

* `getcwd`：获取当前工作目录（等价 `pwd`）

---

### 11、目录分析 / 读取目录内容

#### ① glob（目录分析 / 通配符解析）

```c
int glob(const char *pattern, int flags,
         int (*errfunc)(const char *, int),
         glob_t *pglob);
```

```c
typedef struct {
    size_t   gl_pathc;   // 匹配到的路径数量
    char   **gl_pathv;   // 路径数组
    size_t   gl_offs;    // 预留槽位
} glob_t;
```

* 用于解析：

  * `*`
  * `?`
  * `[]`
* 常用于类 `ls *.c` 的实现

```c
globfree(&globbuf); // 用完必须释放
```

---

#### ② 目录流操作（DIR *）

* `opendir()`（man 3）
* `closedir()`
* `readdir()`
* `rewinddir()`
* `seekdir()`
* `telldir()`

📌 `ls` 核心流程：

1. `opendir`
2. `readdir`
3. `stat / lstat`
4. 格式化输出

---

## 二、系统数据文件和信息

👉 见 **DAY4**

---

## 三、进程环境

👉 见 **DAY5**

