# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <fcntl.h>
# include <unistd.h>

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

    sfd = open(argv[1], O_RDONLY);
    //当open参数中含有O_CREAT时，需要设置文件权限 mode, 最后的文件权限带大小 mode & ~ umask
    dfd =  open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0600);

    //返回的文件描述符 < 0 文件打开失败
    if(sfd < 0)
    {
        perror("open()");
        exit(1);
    } 
    
    if(dfd < 0)
    {
        perror("open()");
        exit(1);
    }

    while(1)
    {
        len = read(sfd, buf, BUFSIZE);
        
        //说明读取错误
        if(len < 0)
        {
            perror("open()");
            break;
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
            ret =  write(dfd, buf + pos, len);
            
            if(ret < 0)
            {
                perror("write()");
                exit(1);
            }        

            len -= ret;
            pos += ret;
        }
        
        
        if(ret < 0)
        {
            perror("write");
            break;
        }
    }

    close(dfd);
    close(sfd);

    return 0;
}