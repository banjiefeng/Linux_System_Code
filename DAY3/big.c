# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h> 
# include <fcntl.h>
# include <unistd.h>

int main (int argc, char * argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage...\n");
        exit(1);
    }
 
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);

    if(fd < 0)
    {
        perror("open()");
        exit(1);
    }

    lseek(fd, 1024LL * 5LL, SEEK_SET);
    write(fd, "", 1);

    exit(0);
}