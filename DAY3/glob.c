# include <stdio.h>
# include <stdlib.h>
# include <glob.h>

# define PAT "/etc/a*.conf"

int errfunc_(const char * path, int eerrno)
{
    puts(path);
    exit(1);
}

int main (int argc, char * argv[])
{
    glob_t globres;

    //0表示成功，非零表示失败 
    int err = glob(PAT, 0, NULL, &globres);
    if(err)
    {
        fprintf(stderr, "ERROR code = %d\n", err);
        exit(1);
    }

    for(int i = 0; i < globres.gl_pathc; i ++)
    puts(globres.gl_pathv[i]);

    globfree(&globres);

    exit(0);
}