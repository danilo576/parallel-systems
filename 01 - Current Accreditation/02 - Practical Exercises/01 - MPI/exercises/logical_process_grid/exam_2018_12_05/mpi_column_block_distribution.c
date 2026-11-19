#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M
#define M 2
#endif
#ifndef L
#define L 1
#endif
#define ROOT 0

/*
 * 05.12.2018, zadatak 2b
 *
 * Matrica B dimenzija m × m podeljena je na blokove od po l uzastopnih
 * kolona, a (m/l)^2 procesa uređeno je kao kvadratna matrica. Proces P0
 * inicijalizuje B. Raspodeliti blokove tako da svaki proces u j-toj koloni
 * mreže dobije kolone j*l, j*l+1, ..., (j+1)*l-1 matrice B.
 * Koristiti grupne operacije i funkcije za kreiranje komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int b[M][M], local_b[M][L];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype column_stride, column_block;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int grid_size = M / L;
    if (M % L != 0 || size != grid_size * grid_size) {
        if (rank == ROOT)
            fprintf(stderr, "M must be divisible by L and the number of processes must be (M/L)^2.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int row = rank / grid_size;
    const int col = rank % grid_size;
    MPI_Comm_split(MPI_COMM_WORLD, row, col, &row_comm);
    MPI_Comm_split(MPI_COMM_WORLD, col, row, &col_comm);

    if (row == 0) {
        if (rank == ROOT)
            for (int i = 0; i < M; i++)
                for (int j = 0; j < M; j++)
                    b[i][j] = i * M + j + 1;

        MPI_Type_vector(M, L, M, MPI_INT, &column_stride);
        MPI_Type_create_resized(column_stride, 0, L * sizeof(int), &column_block);
        MPI_Type_commit(&column_block);
        MPI_Scatter(b, 1, column_block, local_b, M * L, MPI_INT,
                    ROOT, row_comm);
    }

    MPI_Bcast(local_b, M * L, MPI_INT, ROOT, col_comm);

    for (int i = 0; i < M; i++)
        for (int j = 0; j < L; j++)
            printf("P%d: B[%d][%d] = %d\n", rank, i, col * L + j,
                   local_b[i][j]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
