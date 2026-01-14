# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

int main ()
{
    //标准IO与系统调用IO,标准IO会有缓冲区，而文件IO不会有缓冲区
    //因此输出 bbb aaa

    puts("start test!");

    putchar('a');
    write(1, "b", 1);

    putchar('a');
    write(1, "b", 1);

    putchar('a');
    write(1, "b", 1);

    fflush(NULL);  // 刷新所有缓冲区
    exit(0);
}