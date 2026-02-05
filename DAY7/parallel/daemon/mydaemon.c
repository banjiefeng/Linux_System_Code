# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <unistd.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <syslog.h>
# include <errno.h>
# include <string.h>
# include <signal.h>

# define FNAME "/home/jinzheyu/Linux_System_Code/DAY6/daemon/out.tmp"

static FILE * fp;

static int  daemonize()
{
    pid_t pid;
    int fd;

    pid = fork();

    if(pid < 0)
    {
        return -1;
    }

    //父进程退出
    if(pid > 0)
    exit(0);

    //将守护进程的输入输出全部重定向
    fd = open("/dev/null", O_RDWR);

    if(fd < 0)
    {
        return -1;
    }

    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);

    if(fd > 2)
        close(fd);

    //设置当前进程为一个会话进程
    setsid();

    //改变工作目录,防止当前工作目录终止后守护进程异常
    chdir("/");

    return 0;
}

//守护进程终止的时候进行执行
/* 
    1、信号处理函数在执行期间，默认情况下会阻塞同类型信号，
       但其他类型的信号可以打断当前信号处理函数。
    2、如果此时执行函数，假设为SIGINT引发的，当函数执行一半
       时若又产生其他信号，则可能会产生异常，严重时导致内存
       泄漏。
    解决方法：1、执行前屏蔽其他可能被打断的信号，执行后在恢复
              2、使用sigaction
*/
void daemon_exit(int s)
{
    fclose(fp);
    
    syslog(LOG_INFO, "fopen() close");
    
    closelog();
    
}

int main ()
{
    struct sigaction sa;

    sa.sa_handler = daemon_exit;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    // signal(SIGINT, daemon_exit);
    // signal(SIGQUIT, daemon_exit);
    // signal(SIGTERM, daemon_exit);

    openlog("mydaemon", LOG_PID, LOG_DAEMON);

    if(daemonize())
    {
        syslog(LOG_ERR, "daemonize() failed!");
    }
    else
    {
        syslog(LOG_INFO, "daemonize() successed!");
    }

    fp = fopen(FNAME, "w+");

    if(fp == NULL)
    {
        syslog(LOG_ERR, "fopen():%s",strerror(errno));
    }

    for(int i = 0; ; i ++)
    {
        sleep(1);
        fprintf(fp, "%d\n", i);
        fflush(fp);
    }

    exit(0);
}