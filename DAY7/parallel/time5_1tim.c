# include <stdio.h>
# include <time.h>
# include <stdlib.h>

int main ()
{
    time_t end;
    end = time(NULL) + 5;
    long long cnt = 0;

    while (time(NULL) <= end)
    {
        /* code */
        cnt ++;
    }
    
    printf("%lld\n", cnt);


    exit(0);
}