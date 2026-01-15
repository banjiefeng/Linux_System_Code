# include <stdio.h>
# include <stdlib.h>
# include <glob.h>
# include <sys/stat.h>
# include <string.h>

# define PATHSIZE 1024


//判断是否构成回路
static int pathnoloop(const char * path) 
{
    // aaa/bbb/ccc/ddd/. aaa/bbb/ccc/ddd/..
    char * pos;
    //strchr差找字符第一次出现位置
    //strrchr查找字符最后一次出现位置
    pos = strrchr(path, '/');
    if(pos == NULL)
    return 1;

    if(!strcmp(pos + 1, "."))
    return 0;
    else if(!strcmp(pos + 1, ".."))
    return 0;
    else 
    return 1;
}

static __int64_t mydu(const char * path)
{
    //使用static进行优化放入静态区，因为只会在递归发生之前进行使用
    static struct stat statres;
    glob_t globres;
    int flag;
    __int64_t res = 0;
    char nextpath[PATHSIZE];

    //防止查询软链接文件时的权限越界
    flag = lstat(path, &statres);

    if(flag < 0)
    {
        perror("lstate failed");
        exit(1);
    }

    if(!S_ISDIR(statres.st_mode))
    {
        res = statres.st_blocks / 2;
        printf("%s:%lld\n", path, res);
        return res;
    }
    else //目录文件 
    {
        //获取当前目录下所有文件

        //非隐藏文件
        //从soucre拷贝不超过PATHSIZE的字符串到destination
        strncpy(nextpath, path, PATHSIZE);
        
        //注意:文件 . 和 .. 文件
        //追加不超过PATHSIZE的字符串到destination
        strncat(nextpath, "/*", PATHSIZE);

        flag = glob(nextpath, 0, NULL, &globres);
        if(flag)
        {
            perror("glob failed");
            exit(1);
        }
        //隐藏文件
        //从soucre拷贝不超过PATHSIZE的字符串到destination
        strncpy(nextpath, path, PATHSIZE);
        //追加不超过PATHSIZE的字符串到destination
        strncat(nextpath, "/.*", PATHSIZE);
        //使用参数GLOB_APPEND进行追加
        flag = glob(nextpath, GLOB_APPEND, NULL, &globres);
        if(flag)
        {
            perror("glob failed");
            exit(1);
        }
        res += (statres.st_blocks / 2);
        for(int i = 0; i < globres.gl_pathc; i ++)
        {
            //防止.或者..产生回路
            if(pathnoloop(globres.gl_pathv[i]))
            res += mydu(globres.gl_pathv[i]);
        }

        globfree(&globres);

        printf("%s:%lld\n", path, res);

        return res;
    }
} 

int main (int argc, char * argv[])
{
    if(argc < 2)
    {
        perror("Usage...\n");
        exit(1);
    }

    printf("%s:%lld", argv[1], mydu(argv[1]));

    exit(0);
}