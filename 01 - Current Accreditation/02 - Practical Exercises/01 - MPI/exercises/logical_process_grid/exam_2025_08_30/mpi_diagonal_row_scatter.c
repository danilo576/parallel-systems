#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * 30.08.2025, zadatak 6b
 *
 * Napisati MPI program koji kreira komunikator samo za procese na glavnoj
 * dijagonali kvadratne mreže procesa. Master proces inicijalizuje matricu A
 * i raspodeljuje je tako da svaki proces u novom komunikatoru dobije po
 * jednu vrstu. Odštampati identifikatore članova novog komunikatora i
 * sumu elemenata vrste koju je svaki član primio.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int grid_size = (int)sqrt(size);
    if (grid_size * grid_size != size) {
        if (rank == 0)
            fprintf(stderr, "The process count must be a perfect square.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int diag_ranks[grid_size];
    for (int i = 0; i < grid_size; i++)
        diag_ranks[i] = i * grid_size + i;

    MPI_Group world_group, diag_group;
    MPI_Comm diag_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, grid_size, diag_ranks, &diag_group);
    MPI_Comm_create(MCW, diag_group, &diag_comm);

    int row = rank / grid_size;
    int col = rank % grid_size;
    if (row == col) {
        int diag_rank;
        int local_row[grid_size];
        int matrix[grid_size][grid_size];
        MPI_Comm_rank(diag_comm, &diag_rank);

        if (diag_rank == 0) {
            for (int i = 0; i < grid_size; i++)
                for (int j = 0; j < grid_size; j++)
                    matrix[i][j] = i * grid_size + j + 1;
        }

        MPI_Scatter(matrix, grid_size, MPI_INT, local_row, grid_size, MPI_INT,
                    0, diag_comm);

        long long sum = 0;
        for (int i = 0; i < grid_size; i++)
            sum += local_row[i];

        printf("Process %d (diagonal rank %d): row sum = %lld\n",
               rank, diag_rank, sum);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
