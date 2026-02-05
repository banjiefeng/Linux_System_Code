# include <stdio.h>
# include <signal.h>
# include <errno.h>
# include <unistd.h>
# include "mytbf_sigaction.h"
# include <stdlib.h>
# include <sys/time.h>

typedef void (*sighandler_t)(int);

struct tbf_st
{
    /* data */
    int cps;
    int token;
    int burst;
    int pos;
};

static int flag = 0;

static struct tbf_st* tbf_list[MAX_TBF];

//保存旧的sigaction值，方便后续恢复
static struct sigaction oldsigaction;

static int join_tbs_list(struct tbf_st* t)
{
    for(int i = 0; i < MAX_TBF; i ++)
    {
        if(tbf_list[i] == NULL)
        {
            tbf_list[i] = t;
            return i;
        }
    }

    return -1;
}

static int min(int a, int b)
{
    return a > b ? b: a;
}

static void alrm_sigaction(int s, siginfo_t * info, void * unused)
{
    //如果不是内核发出的信号
    if(info->si_code != SI_KERNEL)
    return ;

    for(int i = 0; i < MAX_TBF; i ++)
    {
        if(tbf_list[i] != NULL)
        {
            tbf_list[i]->token += tbf_list[i]->cps;
            if(tbf_list[i]->token > tbf_list[i]->burst)
                tbf_list[i]->token = tbf_list[i]->burst;
        }
    }
}

static void modulo_unload(void)
{
    struct itimerval itv;
 
    for(int i = 0; i < MAX_TBF; i ++)
    free(tbf_list[i]);

    flag = 0;

    //关闭每妙发送信号
    
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;

    //设置定时期每秒发送一次信号
    setitimer(ITIMER_REAL, &itv, NULL);

    //将alarm恢复为原先的函数
    sigaction(SIGALRM, &oldsigaction, NULL);
}

static void modulo_load(void)
{
    // alrm_handler_save = signal(SIGALRM, alrm_handler);
    // alarm(1);

    struct itimerval itv;
    struct sigaction sa;

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    //设置定时期每秒发送一次信号
    setitimer(ITIMER_REAL, &itv, NULL);
 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alrm_sigaction;

    sigaction(SIGALRM, &sa, &oldsigaction);

    //放置钩子函数
    atexit(modulo_unload);
}

tbf_st* tbf_init(int CPS, int BURST)
{
    int ret;
    struct tbf_st* t;
    
    if(!flag)
    {
        modulo_load();
        flag = 1;
    }

    t = (tbf_st *)malloc(sizeof(tbf_st));
    t->token = 0;
    t->burst = BURST;
    t->cps = CPS;
    ret = join_tbs_list(t);

    if(ret < 0)
    {
        printf("list full");
        return NULL;
    }
    t->pos = ret;

    return t;
}

int tbf_fetchtoken(tbf_st* tbf, int size)
{
    //将void * 指针转化为 struct tbf_st *
    struct tbf_st * tbf_res  = tbf;

    if(size <= 0)
    return -EINVAL;
    
    while(tbf_res->token <= 0)
    pause();
    
    int t = min(tbf_res->token, size);
    
    tbf_res->token -= t;

    //返回取得的token数
    return t;
}

int tbf_returntoken(tbf_st* tbf, int size)
{
    //将void * 指针转化为 struct tbf_st *
    struct tbf_st * tbf_res  = tbf;
    if(size <= 0)
    return -EINVAL;

    tbf_res->token += size;

    if(tbf_res->token > tbf_res->burst)
    tbf_res->token = tbf_res->burst;

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

    tbf_list[pos] = NULL;

    free(tbf_res);

    return pos;
}