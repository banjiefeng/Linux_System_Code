# include <stdio.h>
# include <stdlib.h>
# include <sys/mman.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

int main (int argc, char * argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage...\n");
        exit(1);
    }

    //使用mmap将文件映射到进程的虚拟地址空间中
    int fd = open(argv[1], O_RDONLY);
    struct stat st;
    if(fd < 0)
    {
        fprintf(stderr, "Usage...\n");
        exit(1);
    }
    if(fstat(fd, &st) < 0)
    {
        perror("fstat() failed");
        exit(1);
    }
    char * map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, SEEK_SET);

    if(map == MAP_FIXED)
    {
        perror("mmap failed");
        exit(1);
    }
    //由于文件已经映射到了进程的虚拟地址空间，因此可以关闭了
    close(fd);

    int cnt = 0;
    for(int i = 0; i < st.st_size; i ++)
    {
        if(map[i] == 'a')
            cnt ++;
    }

    printf("%d\n", cnt);

    munmap(map, st.st_size);

    return  0;
}