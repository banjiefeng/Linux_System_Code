# include <stdio.h>
# include <stdlib.h>

//atoi将一个串转化为一个整形

int main ()
{
    char str[] = "123456";

    fprintf(stdout, "atoi:%d\n", atoi(str));

    //sprintf将格式化的数据写入字符串
    char buf[128];
    int year = 2026, month = 1, day = 8;

    sprintf(buf, "%d-%d-%d", year, month, day);

    fprintf(stdout, "%s\n", buf);

    return 0;
}