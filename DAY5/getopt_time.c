# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>
# include <string.h>

# define BUFSIZE 1024   

extern char *optarg;//获取选项参数后面的参数
extern int optind;//获取在argv下的下标

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
    char timeformat[BUFSIZE];
    int c;
    FILE * fp = stdout;

    timeformat[0] = '\0';//初始化为空串

    while (1)
    {
        //实现带选项时则需要在相应字符后面添加：，用
        //全局变量optarg获取响应参数
        //-参数位于开头用于指明后面匹配的都是选项参数
        //若遇到非选项参数则反会1
        c = getopt(argc, argv, "-y:mdH:MS");
        
        if(c < 0)
        break;

        switch (c)
        {
            case 1:
            if(fp == stdout)//先入文件的的非选项的传参
            {
                //当获取一个参数后optind会加1,所以要得到上一个需要optind-1
                fp = fopen(argv[optind - 1], "w");
                if(fp == NULL)
                {
                    perror("file failed");
                    fp = stdout;
                }
            }
                break;
            case 'y':
                
                if(!strcmp(optarg, "2"))
                strncat(timeformat, "%y ", BUFSIZE);
                else if(!strcmp(optarg, "4"))
                strncat(timeformat, "%Y ", BUFSIZE);
                else
                fprintf(stderr, "Invalid argument!\n");
                break;
            case 'm':
                strncat(timeformat, "%m ", BUFSIZE);
                break;
            case 'd':
                strncat(timeformat, "%d ", BUFSIZE);
                break;
            case 'H':
                if(!strcmp(optarg, "12"))
                strncat(timeformat, "%I(%P) ", BUFSIZE);
                else if(!strcmp(optarg, "24"))
                strncat(timeformat, "%H ", BUFSIZE);
                else 
                fprintf(stderr, "Invalid argument!");
                break;
            case 'M':
                strncat(timeformat, "%M ", BUFSIZE);
                break;
            case 'S':
                strncat(timeformat, "%S ", BUFSIZE);
                break;
            default:
                break;
        }
    }

    time(&tmt);

    tms = localtime(&tmt);

    strftime(buf, BUFSIZE, timeformat, tms);

    strncat(buf, "\n", BUFSIZE);

    fputs(buf, fp);

    if(fp != stdout)
    fclose(fp);

    exit(0);
}