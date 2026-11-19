#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

/*
 * 15.05.2015. — I kolokvijum, zadatak 3
 * Napisati MPI program koji komunikacijom od tačke do tačke šalje jedan
 * podatak od procesa 0 do svih ostalih procesa u komunikatoru.
 * Broj procesa je stepen dvojke. Svaki primalac prikazuje dobijenu vrednost.
 * Procesi su uređeni u stablo. Za 8 procesa slanja po nivoima su
 * 0 → 1, zatim 0 → 2 i 1 → 3, pa 0 → 4, 1 → 5, 2 → 6 i 3 → 7.
 */
int main(int argc, char **argv)
{
    int rank, size, value;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if ((size & (size - 1)) != 0) {
        if (rank == 0)
            fprintf(stderr, "The number of processes must be a power of two.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
        value = 42;

    for (int step = 1; step < size; step *= 2) {
        if (rank < step) {
            MPI_Send(&value, 1, MPI_INT, rank + step, 0, MCW);
        } else if (rank - step < step) {
            int source = rank - step;
            MPI_Recv(&value, 1, MPI_INT, source, 0, MCW, MPI_STATUS_IGNORE);
            printf("Process %d received %d from process %d.\n", rank, value, source);
        }
    }

    MPI_Finalize();
    return 0;
}
