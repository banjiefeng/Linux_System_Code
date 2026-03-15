# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>

# define PROCNUM 20
# define FNAME "./out.tmp"
# define BUFESIZE 1024

void func_add(void)
{
    FILE * fp;
    char buf[BUFESIZE];
    fp = fopen(FNAME, "r+");

    if(fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }

    int fd = fileno(fp);
    lockf(fd, F_LOCK, 0);
    fgets(buf, BUFESIZE, fp);
    sleep(1);
    fseek(fp, 0, SEEK_SET);

    fprintf(fp, "%d", atoi(buf) + 1);
    //因为fprintf是行缓冲，所以先把东西给刷新写入
    fflush(fp);
    lockf(fd, F_ULOCK, 0);

    fclose(fp);
}

int main ()
{
    int err;
    pid_t procs[PROCNUM];

    for(int i = 0; i < PROCNUM; i ++)
    {
        procs[i] = fork();
        if(procs[i] < 0)
        {
            fprintf(stderr, "fork():%s\n", strerror(err));
            exit(1);
        }

        if(procs[i] == 0)
        {
            func_add();
            exit(0);
        }
    }

    for(int i = 0; i < PROCNUM; i ++)
    wait(NULL);
    

    exit(0);
}