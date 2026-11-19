#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak 3a, 26.03.2019. Izračunati prefiksne sume n celih brojeva
 * pomoću p = n procesa uređenih u hiperkub, gde je n stepen dvojke.
 * Proces P_k treba da sadrži i prikaže sumu prvih k brojeva.
 * U koraku i direktne varijante partner procesa k je k XOR 2^i,
 * a algoritam ima log2(p) koraka. Rešiti i grupnim operacijama.
 * Varijanta sa grupnim operacijama.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    long long *input = NULL, value, prefix = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    if ((process_count & (process_count - 1)) != 0) {
        if (rank == 0)
            fprintf(stderr, "Process count must be a power of two.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        input = malloc((size_t)process_count * sizeof *input);
        for (int index = 0; index < process_count; index++)
            scanf("%lld", &input[index]);
    }
    MPI_Scatter(input, 1, MPI_LONG_LONG_INT, &value, 1, MPI_LONG_LONG_INT, 0, MCW);
    free(input);

    MPI_Exscan(&value, &prefix, 1, MPI_LONG_LONG_INT, MPI_SUM, MCW);
    if (rank == 0)
        prefix = 0;

    printf("P%d: %lld\n", rank, prefix);
    MPI_Finalize();
    return 0;
}
