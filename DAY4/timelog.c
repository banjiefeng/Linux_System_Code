# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>

# define FNAME "time.log"
# define BUFSIZE 128

int main (int argc, char * argv[])
{
    int cnt = 0;
    FILE * fp; 
    char buf[BUFSIZE];
    fp = fopen(FNAME, "a+");
    time_t timeres;
    struct tm *tmres;

    if(fp == NULL)
    {
        perror("file failed!");
        exit(1);
    }
    rewind(fp);
    //EOF是-1，失败或者读取到EOF会返回NULL
    while (fgets(buf, BUFSIZE, fp) != NULL)
        cnt ++;

    while (1)
    {
        time(&timeres);
        tmres = localtime(&timeres);

        fprintf(fp, "%-4d %04d-%02d-%02d %02d:%02d:%02d\n",
                ++cnt,
                tmres->tm_year + 1900,
                tmres->tm_mon + 1,
                tmres->tm_mday,
                tmres->tm_hour,
                tmres->tm_min,
                tmres->tm_sec);

        fflush(fp);
        sleep(1);
    }
    
    
    fclose(fp);

    exit(0);
}