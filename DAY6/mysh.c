# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <glob.h>

# define DELIMS " \t\n"

typedef struct cmd_t
{
    /* data */
    glob_t globres;
}cmd_t;


static void prompt(void)
{
    printf("mysh-0.1$ ");
}

static  parse(char * line, cmd_t  *cmdres)
{
    char * tok;
    int i = 0;
    while(1)
    {
        tok = strsep(&line, DELIMS);
        if(tok == NULL)
        break;
        else if(tok[0] == '\0') 
        continue;

        glob(tok, GLOB_NOCHECK | GLOB_APPEND * i, NULL, &cmdres->globres);
        i = 1; 
    }

}

int main ()
{
    cmd_t cmdres;
    char * linebuf = NULL;
    size_t size = 0;
    __pid_t pid;

    while(1)
    {
        prompt();

        if(getline(&linebuf, &size, stdin) < 0)
        {
            perror("getline failed!");
            continue;
        }
        
        //解析
        parse(linebuf, &cmdres);


        if(0)//内部命令
        {}
        else//外部命令 
        {
            fflush(NULL);
            pid = fork();

            if(pid < 0)
            {
                perror("pid failed");
                exit(1);
            }

            if(pid == 0)
            {
                fflush(NULL);
                //从环境变量里面寻找
                execvp(cmdres.globres.gl_pathv[0], cmdres.globres.gl_pathv);

                perror("execvp()!");

                exit(1);
            }
            else
            {
                wait(NULL);
            }
        }
    }

    exit(0);
}