# 并发

同步：
异步：

异步事件的处理：
1、查询法
2、通知法

kill -l 查询信号

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

信号产生时的处理流程：
1. 信号产生（如 Ctrl+C 产生 SIGINT）
2. 检查该信号是否在 blocked/mask 中
   - 如果在 blocked 中：信号被添加到 pending 集，暂时不递送
   - 如果不在 blocked 中：立即递送给进程（执行 handler）
3. 当从 blocked 中移除信号时，检查 pending 中是否有该信号
   - 如果有：立即递送
   - 如果没有：等待下次产生

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
    
    5、不能从信号处理函数中随意的往外跳 setjmp, longjmp
    因为在响应信号执行信号处理函数之前会将mask和pedding设置为0，在信号处理函数结束后将会mask在设置为1,若直接跳出，将导致mask一直为0,从而后面无法在进行响应
    sigsetjmp和seglongjmp可以跳，因为会保存和恢复跳转前的mask值
    6、信号处理函数在执行期间，默认情况下会阻塞同类型信号，但其他类型的信号可以打断当前信号处理函数。

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

    例：使用单一计时器，利用alarm 或者 setitimer构造一组函数，实现任意数量的计时器

    #include <sys/time.h>

    int setitimer(int which, 
                const struct itimerval *new_value, struct itimerval *old_value);
    ITIMER_REAL    // 真实时间（墙上时钟时间），到期时发送 SIGALRM 信号
    ITIMER_VIRTUAL // 进程虚拟时间（用户态CPU时间），到期时发送 SIGVTALRM 信号
    ITIMER_PROF    // 进程时间（用户态+内核态），到期时发送 SIGPROF 信号
    
    struct itimerval {
        struct timeval it_interval;  // 间隔时间（用于周期性定时器）
        struct timeval it_value;     // 首次到期时间
    };

    struct timeval {
        time_t      tv_sec;     // 秒
        suseconds_t tv_usec;    // 微秒（1/1,000,000 秒）
    };

    pause();
    /**
     * 使调用进程挂起（睡眠），直到收到一个信号为止
     * 返回用户空间后，pause返回-1,errno会被设置为EINTR
     */
    abort();
    /**
     * 发送一个SIGABORT信号，人为的产生一个异常
     */
    system();
    /**
     *  注意：在存在信号的应用程序中使用时要阻塞一个信号，忽略两个信号，否则不能正常使用
     */

    sleep();
    /* 引申使用alarm和pause实现的sleep缺陷
     *  alarm(5);    // 设置定时器
        pause();     // 等待信号
        // 如果在 alarm() 后、pause() 前信号发送了一个SIGALRM信号
        // pause() 会永远等待！
     */

    sleep可以替换的常用的函数
    可以使用`nanosleep()`->避免上述问题
    也可以使用`usleep()`
    `select()`
```
漏桶和令牌桶的实现
slowcat.c 和 slowcat2.c

7、信号集
```c
    信号集类型:sigset_t 可以包含所有的整数
    //把一个信号集清空
    sigemptyset(sigset_t * set)
    //把一个信号集置为全集
    sigfillset(sigset_t * set)
    //添加一个信号到信号集
    sigaddset(sigset_t *set, int signum);
    //从信号集中删除一个信号
    sigdelset(sigset_t * set, int signum);
    //判断信号是否在信号集中
    sigismember(sigset_t * set, int signum)
```

8、信号屏蔽字/信号pedding集的处理
```c
    
    //信号屏蔽字的处理
    sigprocmask(int how, sigset_t *set, sigset_t * oldset);
    SIG_BLOCK:    // 将 set 中的信号添加到当前阻塞信号集中
    SIG_UNBLOCK:  // 从当前阻塞信号集中移除 set 中的信号
    SIG_SETMASK:  // 直接用 set 替换当前阻塞信号集

    //获取这一次响应前的pedding集合
    sigpedding(sigset_t * set);
```

9、拓展
```c
    /**
     * 等待一个信号 类似与pause()
     * 
     *  sigsuspend 等价于下面的原子操作
            //解除阻塞
            sigset_t tmp;
            sigprocmask(SIG_SETMASK, &oledset, &tmp);
            pause();
            sigprocmask(SIG_SETMASK, &tmp, NULL);
            原子的完成下面操作，防止竞态竞争
            1、临时替换进程的信号掩码（阻塞信号集）

            2、挂起进程，等待任意未被阻塞的信号

            3、信号处理函数返回后，恢复原来的信号掩码
     */
    sigsuspend(sigset_t *sigset);//以sigset的mask环境阻塞后，等待信号到来处理后恢复原先的mask环境

    sigaction();//见sigaction说明

    setitimer();

```

10、实时信号
    31个标准信号和31个实时信号
    16个SIGMIN
    15个SIGMAX
    1、实时信号处理相同的信号要排队，多次发送相同的信号按照顺序处理（不同于标准信号:多次发送只记录一次） ,可以通过ulimit -i 修改最大的排队数量
    2、同时到达的实时信号响应有严格的顺序要求
