# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>

int main ()
{
    pid_t pid;

    puts("Begin()");
    
    fflush(NULL);

    pid = fork();

    if(pid == 0)
    {
        execl("/bin/date", "date","+%s", NULL);

        exit(0);
    }
    
    wait(NULL);

    exit(0);
}