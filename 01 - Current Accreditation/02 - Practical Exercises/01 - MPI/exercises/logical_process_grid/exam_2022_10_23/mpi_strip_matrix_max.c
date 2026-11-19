#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#ifndef N
#define N 2
#endif
#ifndef K
#define K 1
#endif
#define Q (N / K)

/*
 * 2a) Pomnožiti matrice A i B dimenzija n × n. Master inicijalizuje
 * matrice i raspodeljuje A u trakama od k vrsta, a B u trakama od k
 * kolona. Broj procesa je (n/k)^2. Svaku traku poslati jednim MPI_Send
 * sa jednim primerkom izvedenog tipa. Za ostalu komunikaciju koristiti
 * grupne operacije. Master prikazuje C, maksimum i rang njegovog vlasnika.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (N % K != 0 || size != Q * Q) {
        if (rank == 0)
            fprintf(stderr, "N must be divisible by K and %d processes are required.\n", Q * Q);
        MPI_Finalize();
        return 1;
    }

    int local_a[K][N], local_b[N][K], local_c[K][K];
    int (*blocks)[K][K] = rank == 0 ? malloc(size * sizeof *blocks) : NULL;

    if (rank == 0) {
        int a[N][N], b[N][N];
        MPI_Datatype row_strip, column_strip;
        MPI_Type_contiguous(K * N, MPI_INT, &row_strip);
        MPI_Type_commit(&row_strip);
        MPI_Type_vector(N, K, N, MPI_INT, &column_strip);
        MPI_Type_commit(&column_strip);

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                a[i][j] = i * N + j + 1;
                b[i][j] = a[i][j];
            }

        for (int dest = 1; dest < size; dest++) {
            MPI_Send(&a[(dest / Q) * K][0], 1, row_strip, dest, 0, MCW);
            MPI_Send(&b[0][(dest % Q) * K], 1, column_strip, dest, 1, MCW);
        }

        for (int i = 0; i < K; i++)
            for (int j = 0; j < N; j++)
                local_a[i][j] = a[i][j];

        for (int i = 0; i < N; i++)
            for (int j = 0; j < K; j++)
                local_b[i][j] = b[i][j];
    } else {
        MPI_Recv(local_a, K * N, MPI_INT, 0, 0, MCW, MPI_STATUS_IGNORE);
        MPI_Recv(local_b, N * K, MPI_INT, 0, 1, MCW, MPI_STATUS_IGNORE);
    }

    for (int i = 0; i < K; i++)
        for (int j = 0; j < K; j++) {
            local_c[i][j] = 0;
            for (int t = 0; t < N; t++)
                local_c[i][j] += local_a[i][t] * local_b[t][j];
        }

    int local_max[2] = {local_c[0][0], rank};
    int global_max[2];
    for (int i = 0; i < K; i++)
        for (int j = 0; j < K; j++)
            if (local_c[i][j] > local_max[0])
                local_max[0] = local_c[i][j];

    MPI_Gather(local_c, K * K, MPI_INT, blocks, K * K, MPI_INT, 0, MCW);
    MPI_Reduce(local_max, global_max, 1, MPI_2INT, MPI_MAXLOC, 0, MCW);

    if (rank == 0) {
        puts("Matrix C:");
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                int owner = (i / K) * Q + j / K;
                printf("%d%c", blocks[owner][i % K][j % K], j == N - 1 ? '\n' : ' ');
            }

        printf("Maximum: %d, process: %d\n", global_max[0], global_max[1]);
    }

    free(blocks);
    MPI_Finalize();
    return 0;
}
