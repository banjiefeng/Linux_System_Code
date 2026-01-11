# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

int main (int argc, char * argv[])
{

    FILE * fp = NULL;

    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");

    if(fp == NULL)
    {
        perror("fopen():");
        exit(1);
    }

    //重新定位文件指针
    fseek(fp, 0, SEEK_END);

   //返回当前文件指针的位置
    fprintf(stdout, "file length: %ld\n", ftell(fp));

    fclose(fp);

    exit(0);
}