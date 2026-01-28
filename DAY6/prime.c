# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
#include <stdbool.h>
bool isPrime(int x)
{
    for(int i = 2; i <= x / i; i ++)
    if(x % i == 0)
    return false;

    return true;
}

int main (int argc, char * argv[])
{
    gid_t gid;
    
    fflush(NULL);

    //交叉分配法
    for(int i = 0; i < 3; i ++)
    {
        gid = fork();

        if(gid < 0)
        {
            perror("fork failed !");
            
            for(int k = 0; k < i; k ++)
            wait(NULL);

            exit(1);
        }

        if(gid == 0)
        {
            for(int j = 500 + i; j <= 600; j += 3)
            {
                if(isPrime(i))
                printf("pid:%d  prime:%d\n", getpid(), j);
            }

            exit(0);

        }
    }

    for(int i = 0; i < 3; i ++)
    wait(NULL);

    exit(0);
}