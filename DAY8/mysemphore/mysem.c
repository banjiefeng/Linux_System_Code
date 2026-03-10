# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include "mysem.h"

struct mysem_st
{
    int value;
    pthread_mutex_t mut;
    pthread_cond_t cond;
};

mysem_st * mysem_init(int inival)
{
    struct mysem_st * me;
    me = (struct mysem_st *)malloc(sizeof(struct mysem_st));
    if(me == NULL)
    return NULL;
    me->value = inival;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);
    return me;
}

//返回添加的值
int mysem_add(mysem_st * sem, int val)
{
    struct mysem_st * me = sem;
    pthread_mutex_lock(&me->mut);
    me->value += val;
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);
    
    return val;
}

int mysem_sub(mysem_st * sem, int val)
{
    struct mysem_st * me = sem;
    pthread_mutex_lock(&me->mut);
    while (me->value < val)
    pthread_cond_wait(&me->cond, &me->mut);

    me->value -= val;

    pthread_mutex_unlock(&me->mut);
    
    return val;
}

int mysem_destory(mysem_st * sem)
{
    struct mysem_st * me = sem;

    pthread_mutex_destroy(&me->mut);
    pthread_cond_destroy(&me->cond);
    free(me);
    return 0;
}