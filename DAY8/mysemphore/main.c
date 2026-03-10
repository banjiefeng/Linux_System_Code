# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <stdbool.h>
# include <pthread.h>
# include <string.h>
# include "mysem.h" 

# define N 4

mysem_st * sem;

void * thr_Prime(void * x)
{
    bool flag = false;

    int n = (int) x;

    for(int i = 2; i <= n / i; i ++)
    if(n % i == 0)
    flag = true;

    if(flag == false)
    printf("%d is a Prime!\n", n);

    mysem_add(sem, 1);
    pthread_exit(NULL);
}

//使用信号量机制保证每次最多只有4个线程在运行

int main (int argc, char * argv[])
{
    pthread_t threads[100];
    int err;

    sem = mysem_init(N);

    if(sem == NULL)
    {
        fprintf(stderr, "mysem_init() failed\n");
        exit(1);
    }

    for(int i = 0; i < 100; i ++)
    {
        /*
            由于每个线程都使用i,由于创建线程需要时间
            将会导致不同的线程使用同一个i
               
            err = pthread_create(threads + i, NULL, thr_Prime, &i);
        */
        mysem_sub(sem, 1);
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

    mysem_destory(sem);


    exit(0);
}