# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <string.h>
# include <unistd.h>

# define THRNUM 4

pthread_mutex_t mutex[THRNUM];

static int next(int n)
{
    if(n + 1 == THRNUM)
    return 0;
    return n + 1;
}

static void *thr_func(void * p)
{
    int n = (int)p;
    int c = 'a' + n;
    while(1)
    {
        pthread_mutex_lock(mutex + n);
        write(1, &c, 1);
        pthread_mutex_unlock(mutex + next(n));
    }
    pthread_exit(NULL);
}

int main ()
{
    int err;
    pthread_t tid[THRNUM];

    for(int i = 0; i < THRNUM; i ++)
    {
        pthread_mutex_init(mutex + i, NULL);
        pthread_mutex_lock(mutex + i);
        
        err = pthread_create(tid + i, NULL, thr_func, (void *)i);

        if(err != 0)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    pthread_mutex_unlock(mutex + 0);

    alarm(1);

    for(int i = 0; i < THRNUM; i ++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}