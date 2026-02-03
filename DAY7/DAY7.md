# 并发

同步：
异步：

异步事件的处理：
1、查询法
2、通知法

## 一、信号
1、信号的概念
    信号是软件中断。
    信号的响应依赖于中断。
1-31标准信号 剩下的为实时信号 kill -l

core 文件：程序运行出错后产生的文件，可以系统默认不会产生，可以通过ulimit -c 最大文件大小来设置

2、signal()
    void (*signal(int signum, void (*func)(int)))(int);
等价于
       typedef void (*sighandler_t)(int);

       sighandler_t signal(int signum, sighandler_t handler);

*信号会打断正在阻塞的系统调用*
对于open 、read 等系统调用当因为等待IO而进入阻塞态时，使用信号会产生errno EINTR导致出现假错，IO异常终止
解决
```c
    do
    {
        sfd = open("FILENAME", O_RDONLY);
        if(sfd < 0)
        {
            if(errno != EINTR)
            {
                perror("open()");
                exit(1);
            }
        }
    }
    while(sfd < 0);

```

3、信号的不可靠
信号的行为不可靠：主要体现在丢失、重置、无序和异步中断等问题。

4、可重入函数
可重入函数是指：一个函数可以被多个任务/线程同时调用，而不会因为共享数据被修改而导致错误结果。

所有的系统调用都是可重入的，一部分库函数也是可重入的，如：memcpy，注意：printf()、malloc()等都不在列表中！

man手册上面函数： _r版本都是可重入的，可以用在信号处理函数中。如:rand()不行，但是rand_r可以

5、*信号的响应过程*
信号是从内核态回到用户态时进行检查（系统调用结束或者中断）
思考：
1、信号从收到到响应有一个不可避免的延迟
    信号是从内核态回到用户态时进行检查

2、如何忽略掉一个信号？
    设置响应的mask位为0

3、标准信号为什么要丢失
    在响应一个信号时，由于每次只能响应一个信号调用响应的处理函数，因此在每次信号响应时将相应mask位设置为0,导致其他相同信号都不会发生响应
    （实时信号并不会丢失）

4、标准信号的响应没有严格的顺序 

    每个进程都有两个位图(32bit)
    情况1：没有信号到来时：调度后进行mask（全1） & pedding（全0）
    情况2：有信号到来时则会在从内核态返回用户态时检查是否有待处理的信号（mask & pedding）

6、信号常用的函数
```c
    kill(); 
    /*
        发送一个信号给进程 
        pid > 0 指定进程 =0 组内广播 =-1 所有允许信号的进程 <-1 指定进程组abs(pid)

        sig =0 时检查一个进程或进程组是否存在，若返回-1也有可能会有进程/进程组存在，此时需要查看errno的值来判断
    */
    raise(); 
    /*
        给当前进程或者线程发信号
        多进程环境下
        等价于:kill(getpid(), sig)
        多线程环境下
        等价于:pthread_kill(pthread_self, sig)
    */
    alarm();
    /**
     * 发送SIGALARM信号给当前进程，以秒为单位计时
     * 
     * 注意：alarm无法实现多任务的计时器
     * alarm(10) alarm(5) 最终会5秒结束
     */
    pause();
    /**
     * 使调用进程挂起（睡眠），直到收到一个信号为止
     * 返回用户空间后，pause返回-1,errno会被设置为EINTR
     * 
     * 引申使用alarm和pause实现的sleep缺陷
     *  alarm(5);    // 设置定时器
        pause();     // 等待信号
        // 如果在 alarm() 后、pause() 前信号发送了一个SIGALRM信号
        // pause() 会永远等待！
     */
    abort();
    system();
```
漏桶和令牌桶的实现
slowcat.c 和 slowcat2.c

7、信号集

8、信号屏蔽字/信号pedding集的处理

9、拓展
```c
    sigsuspend();
    sigaction();
    setitimer();
```
10、实时信号

## 二、线程