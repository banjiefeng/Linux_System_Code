# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <fcntl.h>
# include <sys/stat.h> 

# define FNAME "test3.tmp"

//dup缺陷：非原子操作
//改进：原子操作 dup2
int main (int argc, char * argv[])
{
/*
手动将标准输出重定向到指定文件
    close(1);
    //关闭1后，打开一个文件，将会占据文件描述符1所占据的打开文件表
    fd = open(FNAME, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    puts("Hello World!");    fd = open("FNAME", );
    flush(NULL);
    close(fd);
*/

//dup：返回一个新的文件描述符，指向被复制文件描述符号所指向的进程打开文件表的表项
/* 使用dup进行重定向 缺点：1.无法实现原子操作 2.当fd本身为1时，有问题
    
    fd = open(FNAME, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    close(1);
    dup(fd);//新的文件将会占据1号表项
    close(fd);
*/

/* 使用dup2解决上述烦恼，直接实现重定向
    fd = open(FNAME, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(fd, 1); = (close(1); dup(fd))
    close(fd);
*/
    //文件描述符优先占据进程的打开文件表中
    //较靠前的位置

    int fd;

    //打开一个文件，将会占据文件描述符1所占据的打开文件表
    fd = open(FNAME, O_WRONLY | O_TRUNC | O_CREAT, 0600);

    dup2(fd, 1);
    close(fd);
    puts("Hello World!");


    return 0;
}