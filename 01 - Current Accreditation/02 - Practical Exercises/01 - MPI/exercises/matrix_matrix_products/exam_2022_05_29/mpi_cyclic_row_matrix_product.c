#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define K 4
#define N 2
#define M 2

/*
 * 2.a) Napisati MPI program koji množi matrice A dimenzija k × n i B
 * dimenzija n × m i formira matricu C. Master inicijalizuje A i B.
 * Ako je p broj procesa i k je deljivo sa p, proces Pi dobija vrste A
 * sa indeksima i, i+p, ..., i+k-p. Master šalje svaki blok odjednom,
 * direktno iz matrice A, a celu matricu B šalje svim procesima.
 * Svaki proces računa svoj deo i učestvuje u formiranju rezultata.
 * Koristiti grupne operacije i izvedene tipove podataka.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int a[K][N], b[N][M], c[K][M];
    MPI_Datatype rows_a, rows_c, cyclic_a, cyclic_c;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (K % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "K (%d) must be divisible by the process count (%d).\n", K, size);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_rows = K / size;
    int local_a[local_rows][N], local_c[local_rows][M];

    MPI_Type_vector(local_rows, N, size * N, MPI_INT, &rows_a);
    MPI_Type_create_resized(rows_a, 0, N * sizeof(int), &cyclic_a);
    MPI_Type_commit(&cyclic_a);

    MPI_Type_vector(local_rows, M, size * M, MPI_INT, &rows_c);
    MPI_Type_create_resized(rows_c, 0, M * sizeof(int), &cyclic_c);
    MPI_Type_commit(&cyclic_c);

    if (rank == 0)
    {
        for (int i = 0; i < K; i++)
            for (int j = 0; j < N; j++)
                a[i][j] = i * N + j + 1;

        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                b[i][j] = j >= i ? j - i + 1 : 0;
    }

    MPI_Scatter(a, 1, cyclic_a, local_a, local_rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, N * M, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < local_rows; i++)
        for (int j = 0; j < M; j++)
        {
            local_c[i][j] = 0;
            for (int t = 0; t < N; t++)
                local_c[i][j] += local_a[i][t] * b[t][j];
        }

    MPI_Gather(local_c, local_rows * M, MPI_INT, c, 1, cyclic_c, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        puts("Matrix C:");
        for (int i = 0; i < K; i++)
        {
            for (int j = 0; j < M; j++)
                printf("%d%s", c[i][j], j == M - 1 ? "\n" : " ");
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
