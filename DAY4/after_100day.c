# include <stdio.h>
# include <stdlib.h>
# include <time.h>

# define BUFSIZE 1024   
/*
    -y: year
    -m: month
    -d: day
    -H: hour
    -M: minute
    -S: second
*/
//获取100天以后的时间
int main (int argc, char * argv[])
{
    time_t tmt;
    struct tm * tms;
    char buf[BUFSIZE];

    time(&tmt);

    tms = localtime(&tmt);

    strftime(buf, BUFSIZE, "NOW:%Y-%m-%d", tms);

    puts(buf);

    //利用mktime对于不规格时间进行规格化的特性
    tms->tm_mday += 100;
    mktime(tms);

    strftime(buf, BUFSIZE, "NOW:%Y-%m-%d", tms);

    puts(buf);


    exit(0);
}