# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <string.h>

# define THRNUM 20
# define FNAME "./out.tmp"
# define BUFESIZE 1024

//初始化锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * thr_add(void *)
{
    FILE * fp;
    char buf[BUFESIZE];
    pthread_mutex_lock(&mutex);  // 加锁
    fp = fopen(FNAME, "r+");

    if(fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    fgets(buf, BUFESIZE, fp);
    
    fseek(fp, 0, SEEK_SET);

    fprintf(fp, "%d", atoi(buf) + 1);

    fclose(fp);
    pthread_mutex_unlock(&mutex);  // 解锁
    pthread_exit(NULL);
}

int main ()
{
    int err;
    pthread_t thrs[THRNUM];

    for(int i = 0; i < THRNUM; i ++)
    {
        err =  pthread_create(thrs + i, NULL, thr_add, NULL);
        if(err != 0)
        {
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
        }
    }

    for(int i = 0; i < THRNUM; i ++)
    pthread_join(thrs[i], NULL);
    
    pthread_mutex_destroy(&mutex);

    exit(0);
}