# include <stdio.h>
# include <stdlib.h>
# include <string.h>

int main (int argc, char * argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s ", argv[0]);
        exit(1);
    }

    FILE * stream = NULL;

    // printf("%s\n", argv[1]);

    stream = fopen(argv[1], "r");

    if(stream == NULL)
    {
        perror("fopen failed !");
        exit(1);
    }

    //使用getline时一定要进行初始化
    char * linestr = NULL;
    int buffersize = 0;

    while (1)
    {
        /* code */
        //返回成功读取的字符数量,有换行符则包含换行符
        //会不断的自己reallocate 
        if(getline(&linestr, &buffersize, stream) == -1)//buffersize 是系统分配的缓冲区的大小
        break;
        printf("strlen: %ld\n", strlen(linestr));
        printf("buffersize: %d\n", buffersize);
    }
    
    fclose(stream);

    exit(0);
}
