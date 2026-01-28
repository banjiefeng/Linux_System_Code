# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>

int main (int argc, char * argv[])
{
    pid_t pid;

    printf("[%d]Begin!\n", getpid());

    // **在fork之前刷新所有的流**
    /*
        若不刷新则上面Begin()在终端上
        只会打印1次，若输出重定向到某
        个文件上则会打印两次

        因为终端是行缓冲'\n'会自动刷新，
        而其他设备都是全缓冲需要手动刷
        新
    */

    fflush(NULL);

    pid = fork();

    if(pid > 0)
    {
        printf("My child pid is [%d]\n", pid);
        printf("My pid is [%d]\n", getpid());
        
        //防止子进程变成孤儿进程
        wait(NULL);//为僵尸进程收拾
    }
    else if(pid == 0)
    {
        printf("Pid[%d] is my parent!\n", getppid());
        printf("My pid is [%d]\n", getpid());
    }
    else
    {
        perror("fork failed!");
        exit(1);
    }

    // getchar();

    exit(0);
}