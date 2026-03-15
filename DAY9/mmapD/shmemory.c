# include <stdio.h>
# include <stdlib.h>
# include <sys/mman.h>
# include <unistd.h>
# include <string.h>

# define MEMSIZE 1024

int main ()
{
    //使用mmap共享内存通信
    char * ptr =  mmap(NULL, MEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(ptr == MAP_FAILED)
    {
        perror("failed");
        exit(1);
    }

    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
        //记得释放该共享内存
        munmap(ptr, MEMSIZE);
        exit(1);
    }

    if(pid == 0)
    {
        strcpy(ptr, "Hello!");
        munmap(ptr, MEMSIZE);
        exit(0);
    }
    else
    {
        wait(NULL);
        puts(ptr);
        munmap(ptr, MEMSIZE);
        exit(0);
    }

    return 0;
}