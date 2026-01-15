# include <stdio.h>
# include <stdlib.h>
# include <sys/stat.h>
# include <fcntl.h>

int ftype(const char * fname)
{
    struct stat statres;

    if(stat(fname, &statres) < 0)
    {
        perror("stat()");
        exit(0);
    }

    // man 7 查询inode类型， 使用宏函数
    if(S_ISREG(statres.st_mode))//普通文件
    return '-';
    else if(S_ISDIR(statres.st_mode))//目录文件
    return 'd';
    else if(S_ISCHR(statres.st_mode))//字符设备文件
    return 'c';
    else if(S_ISBLK(statres.st_mode))//块设备文件
    return 'b';
    else if(S_ISFIFO(statres.st_mode))//管道文件
    return 'p';
    else if(S_ISLNK(statres.st_mode))//链接文件
    return 'l';
    else if(S_ISSOCK(statres.st_mode))//socket文件
    return 's';
    else 
    return '?';
}

int main (int argc, char * argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage ..\n");
        exit(1);
    }

    printf("ftype:%c\n", ftype(argv[1]));

    exit(0);
}