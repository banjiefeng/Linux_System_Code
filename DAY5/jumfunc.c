# include <stdio.h>
# include <stdlib.h>
# include <setjmp.h>


jmp_buf flag;

void funC()
{
    printf("Begin!%s\n", __FUNCTION__);

    longjmp(flag, 1);

    printf("End!%s\n", __FUNCTION__);
}

void funB()
{
    printf("Begin!%s\n", __FUNCTION__);

    funC();

    printf("End!%s\n", __FUNCTION__);

}

void funA()
{
    printf("Begin!%s\n", __FUNCTION__);


    int ret = setjmp(flag);

    if(!ret)
    {
        funB();
    }
    else
    {
        printf("ret: %d\n", ret);
        printf("End!%s\n", __FUNCTION__);
    }

}

int main ()
{
    printf("Begin!\n");

    funA();

    printf("End!\n");

    exit(0);//执行exit后可以执行钩子函数
}