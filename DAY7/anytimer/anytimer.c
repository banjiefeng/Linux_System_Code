# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <errno.h>
# include "anytimer.h"

/**
 *
 * job
 * {
 *      time;
 *      at_jobfunc_t;
 *      void * arg;
 *      jid
 *      flag:
 *      {
 *          0: free
 *          1: wait
 *          2: run
 *          3: cancel
 *      }
 * }   
*/

typedef void (*sighandler_t)(int);

typedef enum JFlag
{
    FREE,//空闲
    WAIT,//等待调用
    RUN,//正在调用
    OVER,//调用结束
    CANCEL//任务被取消
}JFlag;

struct anytimer_job_t
{
    /* data */
    int time;
    at_jobfunc_t * job_point;
    void * argv;
    int jid;
    JFlag flag;
};

static struct anytimer_job_t* joblist[MAX_LIST]; 
static volatile int anytimer_inited = 0;
static sighandler_t sighandler_old;

void sighandler(int s)
{
    for(int i = 0; i < MAX_LIST; i ++)
    {
        if(joblist[i] != NULL && joblist[i]->flag == WAIT)
        {
            joblist[i]->time --;
            if(joblist[i]->time == 0)
            {
                joblist[i]->flag = RUN;
                joblist[i]->job_point(joblist[i]->argv);
                joblist[i]->flag = OVER;
            }
        }
    }

    alarm(1);
}
static void modulo_unload(void)
{
    anytimer_inited = 0;
    alarm(0);
    signal(SIGALRM, sighandler_old);
    for(int i = 0; i < MAX_LIST; i ++)
    {
        if(joblist[i] != NULL)
        {
            free(joblist[i]);
            joblist[i] = NULL;
        }
    }
}

static void modulo_load(void)
{
    sighandler_old = signal(SIGALRM, sighandler);
    alarm(1);
    atexit(modulo_unload);
}


void anytimer_init()
{ 
    if(!anytimer_inited)
    {
        anytimer_inited = 1;
        modulo_load();
    }
}


/**
 * return >= 0 成功，返回任务ID
 *        == -EINVAL 失败，参数非法
 *        == -ENOSPC 失败，数组满
 *        == -ENOMEM 失败，内存空间不足
*/
int at_addjob(int sec, at_jobfunc_t *jonp, void * arg)
{
    if(sec < 0)
    {
        return -EINVAL;
    }

    for(int i = 0; i < MAX_LIST; i ++)
    {
        if(joblist[i] == NULL)
        {
            joblist[i] = (struct anytimer_job_t *)malloc(sizeof (struct anytimer_job_t));
            if(joblist[i] == NULL)
            return -ENOMEM;
            joblist[i]->time = sec;
            joblist[i]->job_point = jonp;
            joblist[i]->argv = arg;
            joblist[i]->jid = i;
            joblist[i]->flag = WAIT;
            return i;
        }
    }

    return -ENOSPC;
}

//保证异步操作同步化
/** 
 * return == 0 成功，指定任务已经取消
 *        == -EINVAL 失败，参数非法
 *        == -EBUSY 失败，指定任务已经完成
 *        == -ECANCELED 失败，指定任务重复取消
*/
int at_canceljob(int id)
{
    if(id < 0 || joblist[id] == NULL)
    return -EINVAL;
    else if(joblist[id]->flag = OVER)
    return -EBUSY;
    else if(joblist[id]->flag = CANCEL)
    return -ECANCELED;

    joblist[id]->flag = CANCEL;

    return 0;
}

/** 
 * return == 0 成功，指定任务成功恢复
 *        == -EINVAL 失败，参数非法
 *        == -EBUSY 失败，指定任务不再取消态
*/
int at_resumejob(int id)
{
    if(id < 0 || joblist[id] == NULL)
    return -EINVAL;
    else if(joblist[id]->flag != CANCEL)
    return -EBUSY;

    joblist[id] = WAIT;

    return 0;
}

/**
 * return == 0 成功，指定任务成功释放
 *        == -EINVAL 失败，参数非法
*/
int at_waitjob(int id)
{
    if(id < 0)
    return -EINVAL;

    if(joblist[id] != NULL)
    {
        free(joblist[id]);
        joblist[id] = NULL;
    }

    return 0;
}