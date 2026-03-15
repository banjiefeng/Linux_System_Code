# include "relayer.h"
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <pthread.h>
# include <string.h>

# define BUFSIZE 1024

enum 
{
    STATE_R=1,//有限状态机处于读态
    STATE_W,//写态
    STATE_Ex,//异常处理态
    STATE_T//异常态
};

struct rel_fsm_st
{
    /* data */
    int state;
    int sfd;
    int dfd;
    int len;
    int pos;
    char buf[BUFSIZE];
    char * errstr;
    int fd1_save;
    int fd2_save;
    int64_t count;
};

//给自己看
struct rel_job_st
{
    /* data */
    int job_state;
    int fd1;
    int fd2;
    int fd1_save;
    int fd2_save;
    int pos;
    struct rel_fsm_st fsm12, fsm21; 
};

//job数组
static struct rel_job_st * rel_job[REL_JOBMAX];

pthread_once_t init_once = PTHREAD_ONCE_INIT;

//保证多线程的环境下互斥的访问job数组
static pthread_mutex_t mut_rel_job = PTHREAD_MUTEX_INITIALIZER;

static void fsm_driver(struct rel_fsm_st * fsm)
{
    int ret; 
    switch (fsm->state)
    {
    case STATE_R:
        /* code */
        fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
        if(fsm->len == 0)
        fsm->state = STATE_T;
        else if(fsm->len < 0)
        {
            if(errno == EAGAIN)
                fsm->state = STATE_R;
            else
                {
                    fsm->errstr = "read()";
                    fsm->state = STATE_T;
                }
        }
        else
        {
            fsm->state = STATE_W;
            fsm->pos = 0;
        }
        break;
    case STATE_W:
        ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
        if(ret < 0)
        {
            if(errno == EAGAIN)
                fsm->state = STATE_W;
            else
            {
                fsm->state = STATE_Ex;
                fsm->errstr = "write()";
            }
        }
        else
        {
            fsm->len -= ret;
            fsm->pos += ret;
            if(fsm->len == 0)
                fsm->state = STATE_R;
            else
                fsm->state = STATE_W;
        }
        break;
    case STATE_Ex:
        perror(fsm->errstr);
        fsm->state = STATE_T;
        break;
    case STATE_T:
        break;
    default:
        abort();
        break;
    }
}

static void * thr_relayer(void *p)
{
    while (1)
    {
        pthread_mutex_lock(&mut_rel_job);
        for(int i = 0; i < REL_JOBMAX; i ++)
        {
            if(rel_job[i] != NULL)
            {
                if(rel_job[i]->job_state == STATE_RUNNING)
                {
                    fsm_driver(&rel_job[i]->fsm12);
                    fsm_driver(&rel_job[i]->fsm21);
                    
                    if (rel_job[i]->fsm12.state == STATE_T && rel_job[i]->fsm21.state == STATE_T)
                    rel_job[i]->job_state = STATE_OVER;
                    
                }
            }
        }
        pthread_mutex_unlock(&mut_rel_job);
    }

    pthread_exit(NULL);
}

void module_load(void)
{
    pthread_t tid_relayer;

    int err = pthread_create(&tid_relayer, NULL, thr_relayer, NULL);
    if(err)
    {
        fprintf(stderr, "pthread_create() failed:%s", strerror(err));
        exit(1);
    } 
}

void module_unload(void)
{
    pthread_mutex_lock(&mut_rel_job);
    for(int i = 0; i < REL_JOBMAX; i ++)
    {
        if(rel_job[i] != NULL)
        {
            fcntl(rel_job[i]->fd1, F_SETFL, rel_job[i]->fd1_save);
            fcntl(rel_job[i]->fd2, F_SETFL, rel_job[i]->fd2_save);
            free(rel_job[i]);
        }
    }
    pthread_mutex_unlock(&mut_rel_job);
}

int get_free_pos_unlocked()
{
    pthread_mutex_lock(&mut_rel_job);
    for(int i = 0; i < REL_JOBMAX; i ++)
    if(rel_job[i] == NULL)
    return i;
    pthread_mutex_unlock(&mut_rel_job);

    return -ENOSPC;
}

int rel_addjob(int fd1, int fd2)
{
    struct rel_job_st *me;
    int pos;

    pthread_once(&init_once, module_load);

    me = malloc(sizeof(struct rel_job_st));

    if(me == NULL)
    return -ENOMEM;

    me->fd1 = fd1;
    me->fd2 = fd2;
    me->job_state = STATE_RUNNING;
    //保证用户给的文件描述符以非阻塞
    me->fd1_save =  fcntl(me->fd1, F_GETFL);
    fcntl(me->fd1, F_SETFL, me->fd1_save | O_NONBLOCK);

    me->fd2_save =  fcntl(me->fd2, F_GETFL);
    fcntl(me->fd2, F_SETFL, me->fd2_save | O_NONBLOCK);

    me->fsm12.sfd = fd1;
    me->fsm12.dfd = fd2;
    me->fsm12.state = STATE_R;

    me->fsm21.sfd = fd2;
    me->fsm21.dfd = fd1;
    me->fsm21.state = STATE_R;

    pthread_mutex_lock(&mut_rel_job);
    
    pos = get_free_pos_unlocked();

    if(pos < 0)
    {
        pthread_mutex_unlock(&mut_rel_job);

        fcntl(me->fd1, F_SETFL, me->fd1_save);
        fcntl(me->fd2, F_SETFL, me->fd2_save);
        free(me);

        return -ENOSPC;
    }

    rel_job[pos] = me;
    rel_job[pos]->pos = pos;
    pthread_mutex_unlock(&mut_rel_job);

    return pos;
}

/*
int rel_canceljob(int id);

//收尸
int rel_waitjob(int id, struct rel_stat_st *);

int rel_statjon(int id, struct rel_stat_st *);
*/