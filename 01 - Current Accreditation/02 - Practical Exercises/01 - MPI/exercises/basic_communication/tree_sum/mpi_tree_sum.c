#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * 04.07.2015, zadatak 4(a).
 * Napisati MPI program koji komunikacijom od tačke do tačke sabira n brojeva
 * (n je stepen dvojke) koristeći p = n procesa. Za p = 16 komunikacija se
 * odvija kao na slici. U prvom koraku parovi su (P0, P1), (P2, P3), ...,
 * (P[p-2], P[p-1]), a parcijalne sume se akumuliraju u P0, P2, ..., P[p-2].
 * U drugom koraku parovi su (P0, P2), (P4, P6), ..., (P[p-4], P[p-2]),
 * a parcijalne sume se akumuliraju u P0, P4, ..., P[p-4]. Postupak se
 * ponavlja dok se konačna suma ne akumulira u P0, koji je prikazuje.
 */
int main(int argc, char *argv[])
{
    int rank, size, step;
    long long value, sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    /*
     * A positive power of two has exactly one set bit.
     * Subtracting 1 clears that bit and sets all lower bits,
     * so bitwise AND gives zero only for powers of two.
     * Example: 8 & 7 = 1000 & 0111 = 0000 (power of two),
     *          5 & 4 = 0101 & 0100 = 0100 (not a power of two).
     */
    if ((size & (size - 1)) != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The number of processes must be a power of two.\n");
        MPI_Finalize();
        return 1;
    }

    sum = rank;

    for (step = 1; step < size; step *= 2)
    {
        if (rank % (2 * step) == 0)
        {
            MPI_Recv(&value, 1, MPI_LONG_LONG_INT, rank + step, 0, MCW, MPI_STATUS_IGNORE);
            sum += value;
        }
        else
        {
            MPI_Send(&sum, 1, MPI_LONG_LONG_INT, rank - step, 0, MCW);
            break;
        }
    }

    if (rank == 0)
        printf("Final sum is: %lld\n", sum);

    MPI_Finalize();
    return 0;
}
