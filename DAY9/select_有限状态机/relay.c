# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
#include <sys/select.h>

# define TTY1 "/dev/tty11"
# define TTY2 "/dev/tty12"
# define BUFSIZE 1024

typedef /* ... */ fd_set;

/*完整流程
[初始]
   ↓
select(阻塞) ──有事件─→ [检查就绪fd]
                ↓
         [驱动对应状态机]
                ↓
         [状态可能变化]
                ↓
         [回到循环开始]
                ↓
         [重新布置监视集]
                ↓
         [再次 select]
                ↺
*/

enum 
{
    STATE_R=1,//有限状态机处于读态
    STATE_W,//写态
    STATE_AUTO,
    STATE_Ex,//异常处理态
    STATE_T//异常态
};

struct fsm_st
{
    /* data */
    int state;
    int sfd;
    int dfd;
    int len;
    int pos;
    char buf[BUFSIZE];
    char * errstr;
};


static void fsm_driver(struct fsm_st * fsm)
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

static int max(int a, int b)
{
    return a > b? a: b;
}

void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;
    fd_set rdset, wtset;
    struct fsm_st fsm12;//读左写右
    struct fsm_st fsm21;//读右写左

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);
    
    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while (fsm12.state != STATE_T || fsm21.state != STATE_T)
    {
        /* code */
        //布置监视任务
        FD_ZERO(&rdset);
        FD_ZERO(&wtset);
        if(fsm12.state == STATE_R)
        FD_SET(fsm12.sfd, &rdset);
        if(fsm12.state == STATE_W)
        FD_SET(fsm12.dfd, &wtset);
        
        if(fsm21.state == STATE_R)
        FD_SET(fsm21.sfd, &rdset);
        if(fsm21.state == STATE_W)
        FD_SET(fsm21.dfd, &wtset);
        
        if(fsm12.state < STATE_AUTO || fsm21.state < STATE_AUTO)
        {
            //监视
            if(select(max(fd1, fd2) + 1, &rdset, &wtset, NULL, NULL) < 0)
            {
                if(errno == EINTR)
                continue;
                perror("select()");
                exit(1);
            }
        }

        //查看监视结果
        //如何fd1位于读集，或fd2位于写集合则推动状态机
        if(fsm12.state > STATE_AUTO || FD_ISSET(fd1, &rdset) || FD_ISSET(fd2, &wtset))        
        fsm_driver(&fsm12);
        //如果fd2位于读集合或者fd1位于写集合，推动状态机
        if(fsm21.state > STATE_AUTO ||FD_ISSET(fd2, &rdset) || FD_ISSET(fd1, &wtset))
        fsm_driver(&fsm21);
    }
    

    //恢复文件描述符的状态
    fcntl(fd1, F_SETFD, fd1_save);
    fcntl(fd2, F_SETFD, fd2_save);
}

int main (int argc, char * argv[])
{
    int fd1 = open(TTY1, O_RDWR);
    if(fd1 < 0)
    {
        perror("open failed");
        exit(1);
    }
    
    write(fd1, "TTY1\n", 5);

    //非阻塞方式打开
    int fd2 = open(TTY2, O_RDWR| O_NONBLOCK);
    if(fd2 < 0)
    {
        perror("open failed");
        exit(1);
    }
    write(fd2, "TTY2\n", 5);

    relay(fd1, fd2);


    return 0;
}