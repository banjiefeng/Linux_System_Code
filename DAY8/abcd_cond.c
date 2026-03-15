# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <string.h>
# include <unistd.h>

# define THRNUM 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int num = 0;

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
        pthread_mutex_lock(&mutex);
        while(num != n)
        pthread_cond_wait(&cond, &mutex);

        write(1, &c, 1);
        if(num == 3)
        write(1, "\n", 1);
        num = next(num);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main ()
{
    int err;
    pthread_t tid[THRNUM];

    for(int i = 0; i < THRNUM; i ++)
    {   
        err = pthread_create(tid + i, NULL, thr_func, (void *)i);

        if(err != 0)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }

    alarm(1);

    for(int i = 0; i < THRNUM; i ++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}