# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <dirent.h>

# define PAT "/etc"

int main (int argc, char * argv[])
{
    DIR * dp = NULL;
    struct dirent * cur;

    dp = opendir(PAT);
    if(dp == NULL)
    {
        perror("opendir failed!\n");
        exit(1);
    }

    while ((cur = readdir(dp)) != NULL)
        puts(cur->d_name);
    
    closedir(dp);

    return 0;
}   
