# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>

int main (int argc, char *argv)
{
    FILE *fp = fopen("tmp", "r");

    if(fp == NULL)
    {
        // fprintf(stderr, "fopen() failed! errno = %d\n", errno);
        
        //使用perror更清晰,自动关联errno
        // perror("fopen() failed:");

        //strerror
        fprintf(stderr, "fopen():%s\n", strerror(errno));

        exit(1);
    }

    puts("OK!");

    fclose(fp);

    exit(0);
}