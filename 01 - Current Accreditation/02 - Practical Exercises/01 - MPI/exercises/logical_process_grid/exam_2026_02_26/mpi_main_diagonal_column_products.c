#include <math.h>
#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

enum { ROWS = 2 };

/*
 * 26.02.2026. — 6b
 * Napisati MPI program koji kreira komunikator samo od procesa
 * na glavnoj dijagonali kvadratne mreže procesa. Master P0
 * šalje svakom procesu grupe po jednu kolonu matrice A.
 * U svakom procesu komunikatora prikazati identifikator procesa
 * i proizvod elemenata primljene kolone.
 */
int main(int argc, char **argv)
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
        return 1;
    }

    int diag_ranks[grid_size];
    for (int i = 0; i < grid_size; i++)
        diag_ranks[i] = i * (grid_size + 1);

    MPI_Group world_group, diag_group;
    MPI_Comm diag_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, grid_size, diag_ranks, &diag_group);
    MPI_Comm_create(MCW, diag_group, &diag_comm);

    if (rank / grid_size == rank % grid_size) {
        int diag_rank;
        int a[ROWS][grid_size], local_col[ROWS];
        MPI_Comm_rank(diag_comm, &diag_rank);

        if (diag_rank == 0)
            for (int row = 0; row < ROWS; row++)
                for (int col = 0; col < grid_size; col++)
                    a[row][col] = row * grid_size + col + 1;

        MPI_Datatype column_vector, column;
        MPI_Type_vector(ROWS, 1, grid_size, MPI_INT, &column_vector);
        MPI_Type_create_resized(column_vector, 0, sizeof(int), &column);
        MPI_Type_commit(&column);

        MPI_Scatter(a, 1, column, local_col, ROWS, MPI_INT, 0, diag_comm);

        long long product = 1;
        for (int row = 0; row < ROWS; row++)
            product *= local_col[row];

        printf("Process %d (diagonal rank %d), column %d: product = %lld\n",
               rank, diag_rank, diag_rank, product);
    }

    MPI_Finalize();
    return 0;
}
