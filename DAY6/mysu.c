# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <pwd.h>

//执行前将该文件设置为suid文件
// chmod u+s 可执行文件
//更改当前文件的用户权限为root
//chown root 可执行文件
//否则将无法执行setuid

int main (int argc, char * argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage ...\n");
        exit(1);
    }

    int pid;
    pid  = fork();
    if(pid < 0)
    {
        perror("fork failed!\n");
        exit(1);
    }

    if(pid == 0)
    {
        struct passwd * passres = NULL;

        passres = getpwnam(argv[1]);

        // printf("uid:%d\n", passres->pw_uid);

        setuid(passres->pw_uid);
        
        execvp(argv[2], argv + 2);

        perror("execvp failed !");

        exit(1);
    }
    else 
    {
        wait(NULL);
    }

    return 0;
}