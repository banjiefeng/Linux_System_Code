# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

# define SIZE 1024

//格式./mycp srcfile disfile
int main (int argc, char * argv[])
{
    FILE *srcFile;
    FILE *distFile;
    int n;
    char buf[SIZE];

    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s ./mycp <src_file> <dist_file>", argv[0]);
        exit(1);
    }

    srcFile = fopen(argv[1], "r");

    if(srcFile == NULL)
    {
        perror("srcFile fopen() failed:");
        exit(1);
    }

    distFile = fopen(argv[2], "w");
    
    if(srcFile == NULL)
    {
        perror("distFile fopen() failed:");
        exit(1);
    }

    //每次读取size * nmemb个数据,返回读取的nmem数量，若size=4B,则一个nmem为4B
    while((n = fread(buf, 1, SIZE, srcFile)) > 0)//最好size为一个字节
    {
        fwrite(buf, 1, n, distFile);
    }

    //先关闭依赖别人的对象，在关闭被依赖的对象
    fclose(distFile);
    fclose(srcFile);

    exit(0);
}