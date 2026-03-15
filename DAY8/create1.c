# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <string.h>

static void *func(void *p)
{
    puts("Thread is working!");
    
    pthread_exit(NULL);
}

int main ()
{
    pthread_t tid;
    int err;

    puts("Begin!");

    err = pthread_create(&tid, NULL, func, NULL);

    if(err != 0)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    puts("End!");

    //线程的收尸体
    pthread_join(tid, NULL);

    exit(0);
}