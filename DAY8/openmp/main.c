# include <stdio.h>
# include <stdlib.h>

int main ()
{
#pragma omp parallel sections
{
#pragma omp section
    printf("[%d]Hello", omp_get_thread_num());
#pragma omp section
    printf("[%d]World", omp_get_thread_num());
}

    exit(0);
}