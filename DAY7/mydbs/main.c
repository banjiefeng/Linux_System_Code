# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <fcntl.h>
# include <unistd.h>
# include <errno.h>
# include "mytbf.h"
# include <string.h>

/*
    基于令牌桶的cat
    根据等待的时间确定每次输出的字符
    每妙多输出10个字符
*/

# define CPS 10
# define BURST 100
# define BUFSIZE 1024

int main (int argc, char * argv[])
{
    int sfd;
    char buf[BUFSIZE];
    int len, ret, pos, token;
    tbf_st * tbf;

    if(argc < 2)
    {
        fprintf(stderr, "Usage...\n");
        exit(1);
    }

    //防止信号发出导致处于阻塞态的进程进入运行态
    do
    {
       /* code */
        sfd = open(argv[1], O_RDONLY);
        
        if(sfd < 0)
        {
            if(errno != EINTR)
            {
                perror("open() failed");
                exit(1);
            }
        }

    } while (sfd < 0);

    //初始化令牌桶, 放入CPS 和 BURST
    tbf = tbf_init(CPS, BURST);

    if(tbf == NULL)
    {
        fprintf(stderr, "tbf init failed\n");
        exit(1);
    }

    while(1)
    {   
        do
        {
            token = tbf_fetchtoken(tbf, BUFSIZE);

            if(token < 0)
            {
                fprintf(stderr, "fetch failed:%s\n", strerror(-token));
                exit(1);
            }

            len = read(sfd, buf, token);

            // printf("token: %d, len: %d\n", token, len);

            if(len < 0)
            {
                if(errno != EINTR)
                {
                    perror("read()");
                    break;
                }
            }
        } while (len < 0);        

        if(len < token)
        {
            int t = tbf_returntoken(tbf, token - len);
            if(t < 0)
            fprintf(stderr, "return token failed: %s\n", strerror(-t));
        }

        if(len == 0)
            break;

        //防止读取较少
        pos = 0;
        
        //一定要保证读到多少写到多少
        //目的是保证系统产生信号时不会因为中断导致少写
        while (len > 0)
        {
            /* code */
            ret =  write(1, buf + pos, len);
            
            if(ret < 0)
            {
                //如果因为信号中断导致的写异常
                //则重新进行写入
                if(errno == EINTR)
                continue;
                perror("write()");
                exit(1);
            }        

            len -= ret;
            pos += ret;
        }
    }

    close(sfd);

    return 0;
}