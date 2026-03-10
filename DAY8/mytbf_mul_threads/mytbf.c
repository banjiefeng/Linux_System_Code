# include <stdio.h>
# include <signal.h>
# include <errno.h>
# include <unistd.h>
# include "mytbf.h"
# include <stdlib.h>
# include <pthread.h>
# include <string.h>

struct tbf_st
{
    /* data */
    int cps;
    int token;
    int burst;
    int pos;
    //防止多线程状态下访问出错
    pthread_mutex_t mut;
};

static int flag = 0;

//要以独占存在
static struct tbf_st* tbf_list[MAX_TBF];
static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid_alrm;
//保证模块只被初始化一次
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
struct timespec ts, ts1;

static int get_free_pos_unlocked()
{
    for(int i = 0; i < MAX_TBF; i ++)
    {
        if(tbf_list[i] == NULL)
        {
            return i;
        }
    }

    return -1;
}

static int min(int a, int b)
{
    return a > b ? b: a;
}

static void *thr_alrm(void * p)
{
    ts.tv_sec = 1;
    while(1)
    {
        pthread_mutex_lock(&mut_job);
        for(int i = 0; i < MAX_TBF; i ++)
        {
            if(tbf_list[i] != NULL)
            {
                pthread_mutex_lock(&tbf_list[i]->mut);
                tbf_list[i]->token += tbf_list[i]->cps;
                if(tbf_list[i]->token > tbf_list[i]->burst)
                    tbf_list[i]->token = tbf_list[i]->burst;
                pthread_mutex_unlock(&tbf_list[i]->mut);

            }
        }
        pthread_mutex_unlock(&mut_job);
        //安全函数
        nanosleep(&ts, &ts1);
    }
    pthread_exit(NULL);
}

static void modulo_unload(void)
{
    //回收分发令牌的线程
    pthread_cancel(tid_alrm);
    pthread_join(tid_alrm, NULL);

    for(int i = 0; i < MAX_TBF; i ++)
    if(tbf_list[i] != NULL)
    tbf_destory(tbf_list[i]);

    flag = 0;

    pthread_mutex_destroy(&mut_job);
}

//单独让一个线程每过一秒更新令牌桶
static void modulo_load(void)
{
    int err;
    err = pthread_create(&tid_alrm, NULL, thr_alrm, NULL);
    if(err)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    atexit(modulo_unload);
}

tbf_st* tbf_init(int CPS, int BURST)
{
    int ret;
    struct tbf_st* t;
    
    //为了防止多线程下的多次模块加载
    pthread_once(&init_once, modulo_load);

    t = (tbf_st *)malloc(sizeof(tbf_st));
    if(t == NULL)
        return NULL;
    t->token = 0;
    t->burst = BURST;
    t->cps = CPS;
    pthread_mutex_lock(&mut_job);
    ret = get_free_pos_unlocked();
    if(ret < 0)
    {
        free(t);
        pthread_mutex_unlock(&mut_job);
        return NULL;
    }
    pthread_mutex_init(&t->mut, NULL);
    tbf_list[ret] = t;
    t->pos = ret;
    pthread_mutex_unlock(&mut_job);
    
    return t;
}

int tbf_fetchtoken(tbf_st* tbf, int size)
{
    //将void * 指针转化为 struct tbf_st *
    struct tbf_st * tbf_res  = tbf;

    if(size <= 0)
    return -EINVAL;
    
    //防止多线程情况下访问出错
    //不满足条件时，解锁调度，加锁等待
    pthread_mutex_lock(&tbf_res->mut);
    
    //忙等法取token
    while(tbf_res->token <= 0)
    {
        pthread_mutex_unlock(&tbf_res->mut);
        sched_yield();
        pthread_mutex_lock(&tbf_res->mut);
    }
    
    int t = min(tbf_res->token, size);
    
    tbf_res->token -= t;

    pthread_mutex_unlock(&tbf_res->mut);

    //返回取得的token数
    return t;
}

int tbf_returntoken(tbf_st* tbf, int size)
{
    //将void * 指针转化为 struct tbf_st *
    struct tbf_st * tbf_res  = tbf;
    if(size <= 0)
    return -EINVAL;

    pthread_mutex_lock(&tbf_res->mut);
    tbf_res->token += size;

    if(tbf_res->token > tbf_res->burst)
    tbf_res->token = tbf_res->burst;
    pthread_mutex_unlock(&tbf_res->mut);

    return size;
}

int tbf_destory(tbf_st* tbf)
{
    int pos;
    //将void * 指针转化为 struct tbf_st *
    struct tbf_st * tbf_res  = tbf;

    if(tbf == NULL)
    return -1;

    pos = tbf_res->pos;

    pthread_mutex_lock(&mut_job);
    tbf_list[pos] = NULL;
    pthread_mutex_unlock(&mut_job);
    pthread_mutex_destroy(&tbf_res->mut);
    free(tbf_res);

    return pos;
}