#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak: Napisati MPI program koji izracunava sumu N celih brojeva
 * (N je stepen broja 2) koriscenjem point-to-point komunikacije.
 * U prvom koraku procesi se grupisu u parove (P0, P1), (P2, P3), ...
 * Parcijalne sume akumuliraju se u procesima P0, P2, P4, ...
 * U sledecem koraku parovi su (P0, P2), (P4, P6), ...
 * Postupak se ponavlja dok se konacna suma ne akumulira u procesu P0.
 * Kao na slici, koristi se N procesa i sabiraju brojevi od 0 do N - 1.
 */
int main(int argc, char *argv[])
{
    int rank, size, value, sum, step;
    MPI_Status status;

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
            printf("The number of processes must be a power of two.\n");
        MPI_Finalize();
        return 0;
    }

    sum = rank;

    for (step = 1; step < size; step *= 2)
    {
        if (rank % (2 * step) == 0)
        {
            MPI_Recv(&value, 1, MPI_INT, rank + step, 0, MCW, &status);
            sum += value;
        }
        else
        {
            MPI_Send(&sum, 1, MPI_INT, rank - step, 0, MCW);
            break;
        }
    }

    if (rank == 0)
        printf("Final sum is: %d\n", sum);

    MPI_Finalize();
    return 0;
}
