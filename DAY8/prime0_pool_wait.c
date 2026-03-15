# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <stdbool.h>
# include <pthread.h>
# include <string.h>

# define LEFT 30000
# define RIGHT 30200
# define THRNUM 4

/*
    简单忙等线程池:
    状态: 
    > 0 => 有任务
    = 0 => 无任务
    = -1 => 退出
*/

// 初始化锁
pthread_mutex_t mux_num = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int num = 0;

void * thr_Prime(void * x)
{
    while(1)
    {
        //检查是否可以从线程池中取出数据
        pthread_mutex_lock(&mux_num);
        while(num == 0)
        {
            pthread_cond_wait(&cond, &mux_num);
        }
        
        if(num == -1)
        {
            //一定注意记得解锁，否则程序会死锁
            pthread_mutex_unlock(&mux_num);
            break;
        }

        int n = num;
        num = 0;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mux_num);

        bool flag = false;
    
        for(int i = 2; i <= n / i; i ++)
        if(n % i == 0)
        flag = true;

        if(flag == false)
        printf("[%d]%d is a Prime!\n", (int)x, n);
    }

    pthread_exit(NULL);
}

int main (int argc, char * argv[])
{
    pthread_t threads[THRNUM];
    int err;

    for(int i = 0; i < THRNUM; i ++)
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
 
    for(int i = LEFT; i < RIGHT; i ++)
    {
        pthread_mutex_lock(&mux_num);
        
        //如果任务还在，并没有被抢走
        while(num != 0)
        {
            pthread_cond_wait(&cond, &mux_num);       
        }

        num = i;

        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mux_num);
    }

    //检查num是否被别的进程正在使用
    pthread_mutex_lock(&mux_num);

    //临界资源一定要互斥的进行访问
    while(num != 0)
    {
        pthread_cond_wait(&cond, &mux_num);
    }

    num = -1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mux_num);

    //线程回收
    for(int i = 0; i < THRNUM; i ++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mux_num);
    pthread_cond_destroy(&cond);
    exit(0);
}