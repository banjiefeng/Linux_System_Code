# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

//格式./mycp srcfile disfile
int main (int argc, char * argv[])
{
    FILE *srcFile;
    FILE *distFile;
    int ch;

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

    while(1)
    {
        ch = fgetc(srcFile);

        if(ch == EOF)
            break;
        fputc(ch, distFile);
    }

    //先关闭依赖别人的对象，在关闭被依赖的对象
    fclose(distFile);
    fclose(srcFile);

    exit(0);
}