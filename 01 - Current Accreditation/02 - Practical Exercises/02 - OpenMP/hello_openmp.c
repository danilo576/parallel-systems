#include <omp.h>
#include <stdio.h>

int main(void)
{
    omp_set_dynamic(0);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int broj_niti = omp_get_num_threads();

        printf("ID niti: %d (ukupno niti: %d)\n", id, broj_niti);
    }

    return 0;
}
