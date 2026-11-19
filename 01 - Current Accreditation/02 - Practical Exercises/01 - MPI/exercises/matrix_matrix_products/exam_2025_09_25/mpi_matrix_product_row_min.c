#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

enum { N = 2, K = 4, M = 2 };

/*
 * 25.09.2025. — 1a
 * Napisati MPI program koji množi matrice A (n × k) i B (k × m) i
 * prikazuje matricu C i minimum elemenata svake vrste matrice A.
 * Master inicijalizuje A i B i svakom procesu šalje po k/p kolona A
 * i k/p odgovarajućih vrsta B, gde je p broj procesa i k je deljivo sa p.
 * Sve kolone A namenjene jednom procesu šalju se odjednom, direktno iz A.
 * Isto važi za vrste B. Svi procesi učestvuju u računanju. Rezultate
 * prikazuje proizvoljno izabran proces. Koristiti grupne operacije.
 */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int result_rank = argc > 1 ? atoi(argv[1]) : size - 1;
    if (K % size != 0 || result_rank < 0 || result_rank >= size) {
        if (rank == 0)
            fprintf(stderr, "The process count must divide K and the result rank must be valid.\n");
        MPI_Finalize();
        return 1;
    }

    int a[N][K], b[K][M], c[N][M], row_min[N];
    if (rank == 0) {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < K; j++)
                a[i][j] = i * K + j + 1;

        for (int i = 0; i < K; i++)
            for (int j = 0; j < M; j++)
                b[i][j] = i % M == j;
    }

    int width = K / size;
    int local_a[N][width], local_b[width][M];
    int local_c[N][M] = {0}, local_min[N];
    MPI_Datatype columns, column_block;
    MPI_Type_vector(N, width, K, MPI_INT, &columns);
    MPI_Type_create_resized(columns, 0, width * sizeof(int), &column_block);
    MPI_Type_commit(&column_block);

    MPI_Scatter(a, 1, column_block, local_a, N * width, MPI_INT, 0, MCW);
    MPI_Scatter(b, width * M, MPI_INT, local_b, width * M, MPI_INT, 0, MCW);

    for (int i = 0; i < N; i++) {
        local_min[i] = local_a[i][0];
        for (int t = 0; t < width; t++) {
            if (local_a[i][t] < local_min[i])
                local_min[i] = local_a[i][t];

            for (int j = 0; j < M; j++)
                local_c[i][j] += local_a[i][t] * local_b[t][j];
        }
    }

    MPI_Reduce(local_c, c, N * M, MPI_INT, MPI_SUM, result_rank, MCW);
    MPI_Reduce(local_min, row_min, N, MPI_INT, MPI_MIN, result_rank, MCW);

    if (rank == result_rank) {
        printf("Result on process %d\n", rank);
        puts("C:");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++)
                printf("%d ", c[i][j]);
            putchar('\n');
        }
        printf("Row minima of A:");
        for (int i = 0; i < N; i++)
            printf(" %d", row_min[i]);
        putchar('\n');
    }

    MPI_Finalize();
    return 0;
}
