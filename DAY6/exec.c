# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/*
date +%s
*/

int main ()
{
    puts("Begin");

    // 调用前一定要刷新缓冲区否则缓冲区将会被新的进程映像所替代，从而上述的Begin无法打印

    fflush(NULL);

    //最后一个参数以NULL结尾
    execl("/bin/date", "+%s", NULL);

    puts("End");

    return 0;
}