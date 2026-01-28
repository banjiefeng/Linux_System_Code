# 进程基本知识

## 1、进程标识符 pid
    类型pid_t -> 有符号的16位
    命令ps axf axm
    进程号是顺次向下使用（不同于文件描述符优先使用当前范围内最小的）
    
    ```c
        getpid();//获得当前进程号
        getppid();//获得当前进程的父进程的pid
    ```
## 2、进程的产生
`fork();` -> 调用后产生一个内中断，进入内核态调用fork对应的中断处理程序，由该程序创建子进程的PCB,设置父进程和子进程相应的pid或者ppid
注意理解关键字: duplicating -> 意味着拷贝、克隆一模一样
   fork后父子进程的区别:
1、fork()的返回值不同 0返回给子进程，pid返回给父进程 -1表示出错
2、pid不同
3、ppid不同及父进程的pid不同
4、未决信号（父进程在中断返回后需要响应的信号不会被继承）和文件锁不继承
5、资源利用量清零

`init进程`:是所有进程的祖先进程 pid = 1

调度器的调度策略决定调度顺序，因此需要考虑同步

`fflush的重要性`:
**在fork之前刷新所有的流**
    
    若不刷新则上面Begin()在终端上只会打印1次，若输出重定向到某个文件上则会打印两次

    因为终端是行缓冲'\n'会自动刷新，而其他设备都是全缓冲需要手动刷新
```c
    fflush(NULL);//fork前必须
    fork()
```

    vfork();

## 3、进程的消亡及释放资源
僵尸进程：子进程执行结束，等待父进程回收（父进程未结束），若不回收将会一直占着一个pid
孤儿进程：父进程执行结束，子进程未被回收，子进程变为孤儿进程，被init进程接管，等待init进程回收

```c
    wait(int * status);
    waitpid(pid_t pid, int * status, int options);
    waitid();
    wait3();
    wait4();
```

wait每次只回收一个进程

三种进程的分配方法
1.进程的分块
2.进程的交叉分配
3.池类算法

## 4、exec函数族
exec 系列 函数 用 新的 进程 映象 置换 当前的 进程 映象.

调用前一定要刷新缓冲区否则缓冲区将会被新的进程映像所替代，从而上述的Begin无法打印


```c
    extern char **environ;

    //argv[0]会被忽略，不会有影响

    int execl(const char *path, const char *arg, ...);
    int execlp(const char *file, const char *arg, ...);
    int  execle(const  char  *path,  const  char *arg , ..., char * const
    envp[]);
    int execv(const char *path, char *const argv[]);
    int execvp(const char *file, char *const argv[]);
```

shell工作原理：
shell进程创建一个子进程，子进程执行exec复制进程映像进程调用相应的二进制程序

由于fork是会复制父进程的PCB所以父进程的打开文件表同样被复制，子进程的0、1、2号也指向父进程相应的表项,因此子进程的输出同样输出到shell

## 5、用户权限及组权限


**可以用来实现安全沙箱的资源限制**

每个进程有三种权限 Real UID、Effective UID、Saved UID
Real UID：你实际是谁（登录的用户）

Effective UID：系统认为你是谁（检查权限时用的身份）

Saved UID：备用身份（临时保存的EUID，用于权限恢复）

uid: r e s
gid: r e s

```c
    getuid(); //返回Real的uid
    geteuid(); //返回Effective的uid
    getgid(); //返回Real的gid
    getegid(); //返回Effective的gid
    setuid();// 设置Effective的uid
    setgit();//Effective的gid
    setreuid();//交换real和effective 的uid
    setregid();//交换real和effective 的gid
    seteuid();//更改用户effectived的id
    setegid();
```

1、SUID (u+s) - 设置用户ID
    作用：程序运行时以文件所有者的身份执行
    场景：普通用户执行需要root权限的程序（如 passwd）
    示例：-rwsr-xr-x（所有者执行位变 s）
    设置：chmod u+s 文件

2、SGID (g+s) - 设置组ID
作用：
    文件：运行时以文件所属组的身份执行
    目录：在该目录创建的文件继承目录的所属组
    场景：
    需要组权限的程序
    共享目录（如项目目录，确保文件同组）
    示例：-rwxr-sr-x（组执行位变 s）
    设置：chmod g+s 文件或目录

只有操作系统发现文件（程序）suid位存在，才会设置进程的euid位，从而实现用户以文件所有者的身份操作文件

SUID位决定了程序启动时的 "初始EUID值"，程序运行时通过修改EUID来动态调整权限，Saved UID确保权限切换的安全和可恢复。
```c
    /*
    mysu实现
    1、设置可执行文件的用户为root
    2、设置suid 让其他用户执行该程序时以root用户身份执行
    3、创建子进程，更改当前进程的euid为切换用户uid，接着调用exec函数以切换用户来执行
    */
```

## 6、观磨课：解释器文件
当shell发现当前是#！/bin/bash，会把bash解释器装载到当前shell中，接着由bash解释所有的命令，而发现第一行的#所以不会执行bash,继续执行下面的命令

## 7、system()
    执行shell命令
    相当于 fork + exec("/bi/bash", ...) + wait 的封装

## 8、进程会计
    acct(FILE * file)

## 9、进程时间
    times();//获取进程时间

## 10、守护进程 脱离控制终端

会话：将一个或多个进程组（通常是一个完整的作业或一个用户的登录环境）组合在一起，形成一个独立的“工作单元”，特别是为了终端（控制终端） 的管理。
进程组：将一系列相关的进程（通常是一个命令及其子进程）组合在一起，以便进行统一的作业控制。

守护进程通常设置为一个会话的领导和一个进程组的领导
会话：session,标识sid
```c
    setsid(); 创建一 个会话并且设置进程组的id

    getpgrp(): 返回进程组的组id
```

## 11、系统日志的书写
syslogd服务
```c
    openlog();
    syslog();
    closelog();
```