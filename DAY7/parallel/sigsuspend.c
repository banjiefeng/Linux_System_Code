# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <sys/syscall.h>

typedef void (*sighandler_t)(int);

static void func(int s)
{
    write(1, "!", 1);
}

/*
    信号会在每次中断或者系统调用结束后进行响应
    检查mask集合以及pedding集
    此程序
    发出信号后将进程的mask集相应位置置为1,
    在将对应的mask集置为1,从而实现屏蔽
*/
int main (int argc, char * argv[])
{
    //信号驱动程序的实现

    /*
        //忽略SUGINT 也就是ctrl + c
        signal(SIGINT, SIG_IGN);
    */

    sigset_t set, oledset, saveset;

    //首先将一个信号集置为空
    sigemptyset(&set);
    //添加一个信号到信号集
    sigaddset(&set, SIGINT);

    //在信号来的时候执行函数func
    signal(SIGINT, func);

    /* 保证进入模块状态和离开模块状态整体保持不变
        在进入程序之前保存之前的状态
        因为并不清楚其他程序是否要阻塞
        SIGINT，所以这里解除阻塞，完事后
        在进行恢复
    */
    sigprocmask(SIG_UNBLOCK, &set, &saveset);
    
    //将mask位对应的sigset信号集设置为1的给屏蔽
    sigprocmask(SIG_BLOCK, &set, &oledset);
        
    for(int j = 0; j < 1000; j ++)
    {
        for(int i = 0; i < 5; i ++)
        {
            write(1, "*", 1);
            sleep(1);
        }
        write(1, "\n", 1);
        /* sigsuspend 等价于下面的原子操作
            //解除阻塞
            sigset_t tmp;
            sigprocmask(SIG_SETMASK, &oledset, &tmp);
            pause();
            sigprocmask(SIG_SETMASK, &tmp, NULL);
        */
       sigsuspend(&oledset);
    }

    sigprocmask(SIG_SETMASK, &saveset, NULL);
    
    return 0;
}
