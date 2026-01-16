# define _XOPEN_SOURCE
# include <stdio.h>
# include <stdlib.h>
# include <shadow.h>
# include <crypt.h>
# include <string.h>
# include <unistd.h>

//注意链接库文件-lcrypt

int main (int argc, char * argv[])
{
    char * input_pass, crypted_pass;
    struct spwd * shadowline;

    if(argc < 2)
    {
        fprintf(stderr, "Usage..\n");
        exit(1);
    }

    // 不回显在屏幕上的输入
    input_pass =  getpass("Password:");

    shadowline = getspnam(argv[1]);

    crypted_pass = crypt(input_pass, shadowline->sp_pwdp);

    if(!strcmp(shadowline->sp_pwdp, crypted_pass))
    {
        puts("Successful");
    }
    else
    {
        puts("Failed");
    }

    return 0;
}