高级IO

1、非阻塞IO -- 阻塞IO
阻塞IO：可能会显示假错：ERRNO：EINTR：若当前被阻塞，则当信号到来时会打断 
补充：有限状态机编程
    简单流程：自然流程是结构化的
    复杂流程：自然流程是不是结构化的
数据中继:
左 -> 右 : 读左写右
左 <- 右 ：读右写左

非阻塞IO：EAGAIN，若读的时候没有内容返回EAGAIN 

2、IO多路转接
    监视文件描述符的行为
select();//古老易兼容，有缺陷
以事件为单位来等待文件描述符
```c
    //文件描述符集合
    fd_set fs;

    int select(int nfds, fd_set readfds, fd_set  writefds, fd_set exceptfds, struct timeval timeout);

    //删除某个文件描述符从集合中
    void FD_CLR(int fd, fd_set *set);
    //判断某个文件描述符是否在集合中
    int  FD_ISSET(int fd, fd_set *set);
    //将文件描述符放入集合中
    void FD_SET(int fd, fd_set *set);
    //清空所有文件描述符，可以用于初始化
    void FD_ZERO(fd_set *set);
/*完整流程 select结合状态机
[初始]
   ↓
select(阻塞) ──有事件─→ [检查就绪fd]
                ↓
         [驱动对应状态机]
                ↓
         [状态可能变化]
                ↓
         [回到循环开始]
                ↓
         [重新布置监视集]
                ↓
         [再次 select]
                ↺
*/

```
poll();//可移植
以文件描述符为单位来检查是否有事件发生
```c
    struct pollfd {
        int   fd;         /* file descriptor */
        short events;     /* requested events */
        short revents;    /* returned events */
    };
    //pollfd的数组，返回值可以使用的文件描述符的个数
    //timeout设置小于0表示若无事件发生则一直阻塞
    int poll(struct pollfd *fds, nfds_t nfds, int timeout);

```
epoll();//不可移植,poll升级版
```c
    //分为水平触发ET和边缘触发LT
    //水平触发（Level-Triggered，LT）- 默认模式
    //只要文件描述符可读/可写，就会一直通知，类似于 select 和 poll 的工作方式，如果一次没读完，下次 epoll_wait 还会通知。

    //边缘触发（Edge-Triggered，ET）- 高速模式
    //只在状态发生变化时通知一次，如果一次没读完，不会再次通知，直到有新的数据到达。
    //会创建一个新的 epoll 实例，并返回一个指向该实例的文件描述符。
    epoll_create();
    
    //能向 epoll 实例的兴趣列表中添加项目，注册对特定文件描述符的兴趣。
    epoll_ctl();
    
    //等待 I/O 事件，如果当前没有事件可用，则阻塞调用它的线程。（此系统调用可被看作从 epoll 实例的就绪列表中获取项目。）
    epoll_wait();
```


3、其他读写函数
```c
    //作用：读或者写多个buffer
    readv();
    writev();
```
4、存储映射IO
```c
//1、可以实现文件映射到进程的虚拟地址空间，从而可以实现像访问数组一样操作。2、也可以实现动态内存分配->malloc 3、当采用匿名映射时可以实现进程间的通信
    void *mmap(void addr[.length], size_t length, int prot, int flags, int fd, off_t offset);
         
    int munmap(void addr[.length], size_t length);

```
   
5、文件锁
```c
//放置在并发情况下发生同步问题
    fcntl();
    lockf();
    flock();
```