#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MCW MPI_COMM_WORLD
#define MESSAGE_SIZE 32

/*
 * 2.b) Napisati MPI program koji kreira komunikator od procesa na
 * dijagonali kvadratne mreže procesa. Master novog komunikatora šalje
 * poruku svim ostalim procesima. Svaki proces novog komunikatora
 * prikazuje primljenu poruku.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int grid_size = (int)sqrt(size);
    if (grid_size * grid_size != size)
    {
        if (rank == 0)
            fprintf(stderr, "The process count (%d) must be a perfect square.\n", size);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int row = rank / grid_size;
    int col = rank % grid_size;
    int diag_ranks[grid_size];
    for (int i = 0; i < grid_size; i++)
        for (int j = 0; j < grid_size; j++)
            if (i == j)
                diag_ranks[i] = i * grid_size + j;

    MPI_Group world_group, diag_group;
    MPI_Comm diag_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, grid_size, diag_ranks, &diag_group);
    MPI_Comm_create(MCW, diag_group, &diag_comm);

    if (row == col)
    {
        int diag_rank;
        char message[MESSAGE_SIZE] = {0};

        MPI_Comm_rank(diag_comm, &diag_rank);
        if (diag_rank == 0)
            strcpy(message, "Hello from the diagonal root!");

        MPI_Bcast(message, MESSAGE_SIZE, MPI_CHAR, 0, diag_comm);
        printf("Process %d (diagonal rank %d): %s\n", rank, diag_rank, message);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
