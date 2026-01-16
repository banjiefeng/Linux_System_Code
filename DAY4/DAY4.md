# 文件系统基础 —— 类 `ls`（myls）实现相关知识

## 一、目录与文件

> 目录与文件的基本操作、属性获取等内容  
> 详见 **DAY3**（如 `opendir / readdir / stat / lstat` 等）

---

## 二、系统数据文件与用户信息

Linux 中用户与组的信息由系统统一维护，  
相关 API 可直接从**系统数据库文件**中获取。

---

### 1. `/etc/passwd` —— 用户信息数据库

- 存储 **用户基本信息**
- 所有用户可读

📌 **字段格式：**

用户名:口令占位符:UID:GID:描述:家目录:Shell

📌 **相关结构体（<pwd.h>）：**

```c
struct passwd {
    char   *pw_name;   // 用户名
    uid_t   pw_uid;    // 用户ID
    gid_t   pw_gid;    // 组ID
    char   *pw_dir;    // 家目录
    char   *pw_shell;  // 登录 Shell
};
```
📌 相关函数：
``` c
struct passwd *getpwnam(const char *name); // 通过用户名获取
struct passwd *getpwuid(uid_t uid);        // 通过 UID 获取
```
### 2. `/etc/group` —— 组信息数据库
存储 用户组信息

所有用户可读

📌 相关结构体（<grp.h>）：

```c
struct group {
    char   *gr_name;   // 组名
    gid_t   gr_gid;    // 组ID
    char  **gr_mem;    // 组成员列表
};
```
📌 相关函数：
```c
struct group *getgrnam(const char *name); // 通过组名获取
struct group *getgrgid(gid_t gid);        // 通过 GID 获取
```
### 3. `/etc/shadow` —— 口令影子文件
存储 加密后的用户口令

仅 root 可读

📌 相关结构体（<shadow.h>）：
```c
struct spwd {
    char *sp_namp;   // 用户名
    char *sp_pwdp;   // 加密后的口令
};
```
📌 相关函数：

```c
struct spwd *getspnam(const char *name);
```
📌 口令相关函数：

```c
char *getpass(const char *prompt);          // 读取密码（不回显）
char *crypt(const char *key, const char *salt); // 加密
```
### 三、时间戳与时间处理
Linux 中时间常见有 三种表示方式：

表示方式	类型	适合对象
时间戳	time_t	计算机
时间结构体	struct tm	程序员
字符串	char *	用户

1. 时间戳（time_t）
本质：自 1970-01-01 00:00:00 UTC 起的秒数

```c
#include <time.h>

time_t time(time_t *t);   // man 2

```
2. 时间结构体（struct tm）
```c
struct tm {
    int tm_year;  // 年（从 1900 开始）
    int tm_mon;   // 月（0 ~ 11）
    int tm_mday;  // 日
    int tm_hour;  // 时
    int tm_min;   // 分
    int tm_sec;   // 秒
};
```
📌 时间转换函数（man 3）：

```c
struct tm *gmtime(const time_t *timep);     // UTC 时间
struct tm *localtime(const time_t *timep);  // 本地时间
```
1. 时间规格化（mktime）
对 不规范的 struct tm 自动进位并修正

```c
time_t mktime(struct tm *tm);   // man 3

```
📌 示例：

```c
tm.tm_mday += 100;
mktime(&tm);   // 自动转换为合法日期
```
1. 时间格式化输出（字符串）
```c
size_t strftime(char *s, size_t max,
                const char *format,
                const struct tm *tm);   // man 3
```
📌 示例：
```c
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
```
四、与 myls 的关系总结
在实现类 ls 程序（如 myls -l）时，常用到：

stat / lstat → 获取文件属性

getpwuid → 将 UID 转为用户名

getgrgid → 将 GID 转为组名

localtime + strftime → 显示文件修改时间

📌 一句话总结：

time_t 用来算
struct tm 用来改
strftime 用来给人看

/etc/passwd 和 /etc/group
用来把 UID / GID 翻译成人类可读信息