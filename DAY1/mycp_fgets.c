# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

# define SIZE 1024

//格式./mycp srcfile disfile
int main (int argc, char * argv[])
{
    FILE *srcFile;
    FILE *distFile;
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

    //每次读取SIZE-1个字节,若读完后返回NULL
    while(fgets(buf, SIZE, srcFile) != NULL)
    {
        fputs(buf, distFile);
    }

    //先关闭依赖别人的对象，在关闭被依赖的对象
    fclose(distFile);
    fclose(srcFile);

    exit(0);
}