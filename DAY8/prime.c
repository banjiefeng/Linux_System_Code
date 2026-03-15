# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <stdbool.h>
# include <pthread.h>
# include <string.h>
 

void * thr_Prime(void * x)
{
    bool flag = false;

    int n = (int) x;

    for(int i = 2; i <= n / i; i ++)
    if(n % i == 0)
    return true;

    if(flag == false)
    printf("%d is a Prime!\n", n);

    pthread_exit(NULL);
}

int main (int argc, char * argv[])
{
    pthread_t threads[100];
    int err;

    for(int i = 0; i < 100; i ++)
    {
        /*
            由于每个线程都使用i,由于创建线程需要时间
            将会导致不同的线程使用同一个i
               
            err = pthread_create(threads + i, NULL, thr_Prime, &i);
        */
       
        err = pthread_create(threads + i, NULL, thr_Prime, (void *)i);
        if(err < 0)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }
 
    //线程回收
    for(int i = 0; i < 100; i ++)
    {
        pthread_join(threads[i], NULL);
    }

    exit(0);
}