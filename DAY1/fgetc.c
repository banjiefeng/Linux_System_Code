# include <stdio.h>
# include <stdlib.h>
# include <errno.h>

int main (int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    FILE * fp = NULL;
    int count;

    fp = fopen(argv[1], "r");

    if(fp == NULL)
    {
        perror("fopen() failed:");
        exit(1);
    }

    while (fgetc(fp) != EOF)
    {
        count ++;
    }
    
    fprintf(stdout, "count: %d\n", count);

    exit(0);
}