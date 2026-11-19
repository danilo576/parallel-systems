#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * 19.01.2026, deo b)
 * Procesi u MPI programu imaju podatke kao na slici a). Navesti dve MPI
 * funkcije za komunikaciju, sa celim pozivima i argumentima, koje razmenjuju
 * podatke kao na slici b). Početni nizovi b1 procesa P0, P1, P2 i P3 su
 * {0, 6}, {2, 4}, {9, 3} i {5, 7}. Posle razmene njihovi nizovi b2 treba
 * da budu {5, 7}, {0, 6}, {2, 4} i {9, 3}, dok b1 zadržava svoje vrednosti.
 */
int main(int argc, char *argv[])
{
    const int initial[4][2] = {{0, 6}, {2, 4}, {9, 3}, {5, 7}};
    int rank, size, b2[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (size != 4)
    {
        if (rank == 0)
            fprintf(stderr, "Run with exactly four processes.\n");
        MPI_Finalize();
        return 1;
    }

    int b1[2] = {initial[rank][0], initial[rank][1]};
    int destination = (rank + 1) % size;
    int source = (rank + size - 1) % size;

    if (rank % 2 == 0)
    {
        MPI_Recv(b2, 2, MPI_INT, source, 0, MCW, MPI_STATUS_IGNORE);
        MPI_Send(b1, 2, MPI_INT, destination, 0, MCW);
    }
    else
    {
        MPI_Send(b1, 2, MPI_INT, destination, 0, MCW);
        MPI_Recv(b2, 2, MPI_INT, source, 0, MCW, MPI_STATUS_IGNORE);
    }

    printf("P%d: b1 = {%d, %d}, b2 = {%d, %d}\n",
           rank, b1[0], b1[1], b2[0], b2[1]);

    MPI_Finalize();
    return 0;
}
