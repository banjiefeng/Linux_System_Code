# include <stdio.h>
# include <stdlib.h>
# include <signal.h>

//防止编译器在看不见的地方优化loop,强制每次从内存读取最新值
static volatile int loop = 1;

static void alarm_handler(int s)
{
    loop = 0;
}

int main ()
{
    long long cnt = 0;

    alarm(5);

    signal(SIGALRM, alarm_handler);

    while(loop)
    cnt ++;

    printf("%lld\n", cnt);

    return 0;
}