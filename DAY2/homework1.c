# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

# define BUFSIZE 1024

//定位到某一行的起始位置，若不存在则返回-1
int locate_row_pos(int fd, int row)
{
    lseek(fd, 0, SEEK_SET);
    int t = 1;
    off_t pos = 0;
    char x;
    
    if (row == 1) return 0;  // 第1行从文件开头开始
    
    while (1)
    {
        int u = read(fd, &x, 1);
        if (u <= 0)
            return -1;  // 文件不足row行
        
        if (x == '\n')
        {
            t++;
            if (t == row)
                return pos + 1;  // 返回第row行的起始位置
        }
        pos++;
    }
}

int main (int argc, char * argv[])
{

    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    //三种方法
    int fd_read = open(argv[1], O_RDWR);
    if(fd_read < 0)
    {
        perror("open for read failed !");
        exit(1);
    }

    int fd_write = open(argv[1], O_RDWR);
    if(fd_write < 0)
    {
        perror("open for write failed !");
        exit(1);
    }

    int row1 = 11, row2 = 10, pos1 = 0, pos2 = 0, pos3 = 0, ret, buf[BUFSIZE];
    
    //找到第10行的位置
    pos2 = locate_row_pos(fd_write, row2);

    if(pos2 < 0)
    {
        fprintf(stderr, "文件不足%d行\n", row2);
        close(fd_read);
        close(fd_write);
        exit(1);
    }

    //从第11行开始读取，一次进行写入
    //检测到整个文件全部读取结束则记录下
    //当前的行号，并使用ftruncate从文件开
    //头阶段到文件的最后一行的前一行
    
    pos1 = locate_row_pos(fd_read, row1);
    if(pos1 < 0)
    {
        fprintf(stderr, "文件不足%d行\n", row1);
        close(fd_read);
        close(fd_write);
        exit(1);
    }

    // 定位fd_read到第11行开始处
    lseek(fd_read, pos1, SEEK_SET);
    // 定位fd_write到第10行开始处
    lseek(fd_write, pos2, SEEK_SET);

    while(1)
    {
        int len = read(fd_read, buf, BUFSIZE);
        if(len < 0)
        {
            perror("read failed");
            break;
        }

        //读到文件末尾，截断并返回
        if(len == 0)
        {
            ftruncate(fd_write, pos2);
            break;
        }
        
        pos3 = 0;
        while (len > 0)
        {
            /* code */
            ret = write(fd_write, buf + pos3, len);
            if(ret < 0)
            {
                perror("write failed");
                break;
            }
            len -= ret;
            pos3 += ret;
            pos2 += ret;
        }
    }

    close(fd_write);
    close(fd_read);
    
    exit(0);
}