# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>

static void func(int s)
{
    write(1, "!", 1);
}

int main (int argc, char * argv[])
{
    /*
    //忽略SUGINT 也就是ctrl + c
    signal(SIGINT, SIG_IGN);
    */

    //在信号来的时候执行函数func
    signal(SIGINT, func);

    for(int i = 0; i < 10; i ++)
    {
        write(1, "*", 1);
        sleep(1);
    }

    return 0;
}
