二、线程

1、线程的概念
一个正在运行的函数
posix线程是一套标准，而不是实现
openmp线程
线程标识: phtread_t 类比 pid_t =>但是可能是int、struct等，但是在linux下为int，与pid共用数字

```c
    //用于比较两个线程id是否相同，因为不同系统pthread_t类型可能不相同
    int pthread_equal(pthread_t p1, pthread_t p2);

    //返回当前线程的线程标识符
    pthread_t pthread_self(void);
```

2、线程的一般使用
(1) 线程的创建
```c
    // 成功返回0,失败返回errno
    int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,  
    void *(*start_routine)(void*), void *restrict arg);

    //编译时链接pthread

```
`线程的调度取决于调度器的策略`
(2) 线程的终止
    3种方式：
     1）、 线程从启动例程返回，返回值就是线程的退出码 （正常终止）
     2）、 线程可以被同一进程中的其他线程取消 （异常终止）
     3）、 线程调用pthread_exit()函数 （正常终止）
```c
    //线程终止函数，推荐使用，可以在使用后自动的进行线程的清理
    pthread_exit(void *)

    //线程的收尸 类似于wait()
    pthread_join(pthread_t thread, void **retval);
```
(3) 线程的清理 类似于atexit() 钩子函数
线程调用 pthread_exit() 退出；

线程被其他线程取消（pthread_cancel），并且其取消状态为允许取消（PTHREAD_CANCEL_ENABLE），取消类型为延迟取消（PTHREAD_CANCEL_DEFERRED）；

线程调用 pthread_cleanup_pop(1)，显式执行清理函数。
`这两个函数一定是成对出现的`
```c
    //这两个函数用于注册和弹出线程清理处理程序（thread cleanup handlers）。
       void pthread_cleanup_push(void (*routine)(void *), void *arg);//将清理函数 routine 及其参数 arg 压入当前线程的清理栈；
       
       void pthread_cleanup_pop(int execute);//从栈顶弹出一个清理函数，如果 execute 非零，则执行该清理函数。

``` 
(4) 线程的取消
给一个线程发送取消请求
```c
    pthread_cancel();
    /*
    取消有两种状态：允许和不允许
    允许取消又分为：异步cancel和推迟cancel（默认）-> 推迟至cancel点在取消
    cancel点:POSIX定义的cancel点，都是可能引发阻塞的系统调用

    例：
    fd1 = open();//cancel点

    pthread_cleanup_push();

    fd2 = open();//cancel点
    pthread_cleanup_push()
    当在执行fd1 = open()后一个线程发送了取消，那么会推迟到fd2 = open()才会进行线程取消
    */

    //设置是否允许取消
    int pthread_setcancelstate(int state, int *oldstate);
    
    //设置取消状态 异步cancel和推迟cancel
    int pthread_setcanceltype(int type, int *oldtype);

    //设置取消点
    void pthread_testcancel(void);
```

(5)线程分离
```c
    //不想管的线程，让其自动消亡，main线程不进行收尸
    pthread_detach(pthread_t thread);
```

3、线程的同步

**互斥量**: 一般都是查询法  忙等
```c
    pthread_mutex_t  mutex //类型
    pthread_mutex_init();
    pthread_mutex_destroy();
    pthread_mutex_lock();//阻塞等
    pthread_mutex_trylock();//非阻塞等待
    pthread_mutex_unlock();
    pthread_once()//保证模块的单次初始化
```

**条件变量**:  可以实现通知法
```c
    pthread_cond_t 
    pthread_cond_init()
    pthread_cond_destroy();
    pthread_cond_broadcase();//所有都叫醒
    pthread_cond_signal();//叫醒一个
    //解锁且等待，等待其他信号发通知,当线程苏醒后必定又被加锁
    pthread_cond_wait();//死等
    pthread_cond_timewai();//时间等待
```
**信号量**:
    使用互斥量加条件变量来实现
    使用信号量实现读写锁
    读锁：类似于共享锁
    写锁：互斥锁

4、线程属性
线程同步的属性
1.线程属性
```c
    pthread_attr_t p_attr;//线程属性的结构体，可以通过在pthread_create()中添加从而设置线程的属性
    
    //初始化线程属性
    int pthread_attr_init(pthread_attr_t *attr);
    
    //销毁线程属性
    int pthread_attr_destroy(pthread_attr_t *attr);

    //可以通过下面函数来设置线程属性
    pthread_attr_setstack();
    pthread_attr_setdetachstate();
    pthread_attr_setstackaddr();
    ....
```
2.互斥量属性
```c
    pthread_mutexattr_init();
    pthread_mutexattr_destory();
    pthread_mutexattr_getpshared();//p只得是是否跨进程起作用
    pthread_mutexattr_setpshared();
    clone();//创建一个子进程，根据自己的需求来设置不同的namespace
```

   互斥量的类型
```c
    /*
        PTHREAD_MUTEX_NORMAL
        PTHREAD_MUTEX_ERRORCHCK
        PTHREAD_MUTEX_RECURSIVE
        PTHREAD_MUTEX_DEFAULT
    */

    //获取互斥量的类型
    pthread_mutexattr_gettype();
    pthread_mutexattr_settype();
``` 
条件变量的属性
```c
    pthread_condattr_init();
    pthread_condattr_destory();
    .... 查阅手册
```

5、 可重入
    多线程中的IO -> 若没有加锁版本则后面会添加_unlocked

6、线程与信号的关系
    每一个线程都有自己的mask位图和pedding位图
    而进程为单位只有pedding位图
    当信号响应时会根据当前正在苏醒后正在执行的线程的mask和pedding和进程自身的pedding得出应该响应哪些
```c
    //设置线程的mask
    pthread_sigmask();
    pthread_kill();//以线程为单位发送信号
````
    线程与fork之间的关系
    根据不同的标准有不同的实现

两种线程标准: posix标准、openmp标准