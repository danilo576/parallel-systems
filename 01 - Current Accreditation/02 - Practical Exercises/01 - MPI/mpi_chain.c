#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak: Napisati program koji uzima podatke od nultog procesa i salje ih
 * svim drugim procesima tako sto proces i treba da primi podatke i posalje ih
 * procesu i + 1, sve dok se ne stigne do poslednjeg procesa. Unos podataka
 * se zavrsava nakon sto se prenese negativna vrednost podatka.
 */
int main(int argc, char *argv[])
{
    int rank, size, value;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    do
    {
        if (rank == 0)
        {
            printf("Enter an integer (negative to exit):\n");
            fflush(stdout);
            scanf("%d", &value);
        }
        else
        {
            MPI_Recv(&value, 1, MPI_INT, rank - 1, 0,
                     MCW, &status);
            printf("Process %d received %d\n", rank, value);
        }

        if (rank < size - 1)
        {
            MPI_Send(&value, 1, MPI_INT, rank + 1, 0,
                     MCW);
        }

    } while (value >= 0);

    MPI_Finalize();
    return 0;
}
