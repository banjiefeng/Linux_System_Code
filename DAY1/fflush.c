# include <stdio.h>
# include <stdlib.h>

int main ()
{

    printf("hhhhhh");

    //强制刷新行缓冲
    fflush(stdout);

    while (1)
    {
        /* code */
    }
    
    printf("llllll");

    return 0;
}