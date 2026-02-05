# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <fcntl.h>
# include <unistd.h>
# include <errno.h>

# define BUFSIZE 1024

int main (int argc, char * argv[])
{
    int sfd, dfd;
    char buf[BUFSIZE];
    int len, ret, pos;

    if(argc < 3)
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
    
    //当open参数中含有O_CREAT时，需要设置文件权限 mode, 最后的文件权限带大小 mode & ~ umask
    do
    {
       dfd =  open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0600);

        if(dfd < 0)
        {
            if(errno != EINTR)
            {
                perror("open() failed");
                close(sfd);
                exit(1);
            }
        }

    } while (dfd < 0);

    while(1)
    {
        do
        {
            len = read(sfd, buf, BUFSIZE);
            if(len < 0)
            {
                if(errno != EINTR)
                {
                    perror("open()");
                    break;
                }
            }
        } while (len < 0);        

        if(len == 0)
            break;

        //防止读取较少
        pos = 0;
        
        //一定要保证读到多少写到多少
        //目的是保证系统产生信号时不会因为中断导致少写
        while (len > 0)
        {
            /* code */
            ret =  write(dfd, buf + pos, len);
            
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

    close(dfd);
    close(sfd);

    return 0;
}