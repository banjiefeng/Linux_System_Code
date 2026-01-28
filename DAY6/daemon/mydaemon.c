# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <unistd.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <syslog.h>
# include <errno.h>
# include <string.h>

# define FNAME "/home/jinzheyu/Linux_System_Code/DAY6/daemon/out.tmp"

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

int main ()
{
    FILE * fp;

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

    fclose(fp);
    closelog();

    exit(0);
}