# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "anytimer.h"
# include <signal.h>

static void f3(void * p)
{
    printf("f3():%s\n", p);
}

static void f2(void * p)
{
    printf("f2():%s\n", p);
}

static void f1(void * p)
{
    printf("f1():%s\n", p);
}

int main (int argc, char * argv)
{
    int job1, job2, job3;
    anytimer_init();
    puts("Begin!");

    job1 = at_addjob(5, f1, "aaa");
    
    if(job1 < 0)
    {
        fprintf(stderr, "at_addjob():%s\n", strerror(-job1));
        exit(1);
    }

    job2 = at_addjob(2, f2, "bbb");
    
    if(job2 < 0)
    {
        fprintf(stderr, "at_addjob():%s\n", strerror(-job1));
        exit(1);
    }

    job3 = at_addjob(7, f1, "ccc");

    if(job3 < 0)
    {
        fprintf(stderr, "at_addjob():%s\n", strerror(-job1));
        exit(1);
    }

    puts("End!");
    
    while(1)
    {
        pause();
    }

    wait(job1);
    wait(job2);
    wait(job3);

    exit(0);
}