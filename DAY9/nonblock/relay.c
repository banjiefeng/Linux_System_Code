# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>

# define TTY1 "/dev/tty11"
# define TTY2 "/dev/tty12"
# define BUFSIZE 1024

enum 
{
    STATE_R=1,//有限状态机处于读态
    STATE_W,//写态
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

void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;
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
        fsm_driver(&fsm12);
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