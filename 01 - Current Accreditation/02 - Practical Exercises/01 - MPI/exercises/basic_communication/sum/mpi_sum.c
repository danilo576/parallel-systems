#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak: Napisati program koji nalazi sumu n celih brojeva koriscenjem
 * point-to-point komunikacije tako da svaki proces ucestvuje u sumiranju.
 * U ovom resenju sabiraju se brojevi od 1 do n, uz n procesa.
 */
int main(int argc, char *argv[])
{
    int sum = 0;
    int rank, size, value;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (size == 1)
    {
        sum = 1;
        printf("Final sum is: %d\n", sum);
    }
    else if (rank == 0)
    {
        value = 1;
        MPI_Send(&value, 1, MPI_INT, rank + 1, 0, MCW);
    }
    else if (rank < size - 1)
    {
        MPI_Recv(&value, 1, MPI_INT, rank - 1, 0, MCW, &status);
        sum = rank + 1 + value;
        MPI_Send(&sum, 1, MPI_INT, rank + 1, 0, MCW);
    }
    else
    {
        MPI_Recv(&value, 1, MPI_INT, rank - 1, 0, MCW, &status);
        sum = rank + 1 + value;
        printf("Final sum is: %d\n", sum);
    }

    MPI_Finalize();
    return 0;
}
