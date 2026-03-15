# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <string.h>

static void clean_func(void *p)
{
    puts(p);    
}

static void *func(void *p)
{
    puts("Thread is working!");
    
    pthread_cleanup_push(clean_func, "cleanup:1");
    pthread_cleanup_push(clean_func, "cleanup:2");
    pthread_cleanup_push(clean_func, "cleanup:3");
    puts("over");

    pthread_exit(NULL);

    //必须要有，负责将无法通过编译
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
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

    //线程的收尸体
    pthread_join(tid, NULL);

    puts("End!");

    exit(0);
}