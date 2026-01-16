# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <pwd.h>

int main (int argc, char * argv[])
{
    struct passwd *passwdres;

    if(argc < 2)
    {
        fprintf(stderr, "Usage....\n");
        exit(1);
    }

    passwdres = getpwuid(atoi(argv[1]));

    printf("%s\n", passwdres->pw_name);

    exit(0);
}