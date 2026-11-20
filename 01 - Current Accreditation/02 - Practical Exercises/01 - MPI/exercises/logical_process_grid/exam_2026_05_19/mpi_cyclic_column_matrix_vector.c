#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define N 2
#define K 6

/*
 * 19.05.2026, zadatak 6a.
 * Napisati MPI program koji množi matricu A dimenzija n x k i vektor b.
 * Master inicijalizuje A i b. Broj kolona k je deljiv brojem procesa p.
 * Proces Pi dobija kolone A i elemente b sa indeksima i, i+p, ..., i+k-p.
 * Svaki proces dobija sve svoje kolone odjednom i sve svoje elemente b
 * odjednom. Svi procesi računaju doprinose rezultujućem vektoru c.
 * Koristiti grupne operacije, osim za raspodelu b, gde se koristi
 * komunikacija od tačke do tačke.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int a[N][K], b[K];
    int local_result[N] = {0}, result[N];
    MPI_Datatype matrix_type, scatter_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The number of columns must be divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_cols = K / size;
    int local_a[N][local_cols], local_b[local_cols];

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < K; j++)
                a[i][j] = i * K + j + 1;
        for (int j = 0; j < K; j++)
            b[j] = j + 1;
    }

    MPI_Type_vector(N * local_cols, 1, size, MPI_INT, &matrix_type);
    MPI_Type_create_resized(matrix_type, 0, sizeof(int), &scatter_type);
    MPI_Type_commit(&scatter_type);
    MPI_Scatter(a, 1, scatter_type, local_a, N * local_cols, MPI_INT, 0, MCW);

    if (rank == 0)
    {
        MPI_Datatype vector_type;
        MPI_Type_vector(local_cols, 1, size, MPI_INT, &vector_type);
        MPI_Type_commit(&vector_type);

        for (int dest = 1; dest < size; dest++)
            MPI_Send(&b[dest], 1, vector_type, dest, 0, MCW);

        for (int j = 0; j < local_cols; j++)
            local_b[j] = b[j * size];
    }
    else
    {
        MPI_Recv(local_b, local_cols, MPI_INT, 0, 0, MCW, MPI_STATUS_IGNORE);
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < local_cols; j++)
            local_result[i] += local_a[i][j] * local_b[j];

    MPI_Reduce(local_result, result, N, MPI_INT, MPI_SUM, 0, MCW);

    if (rank == 0)
    {
        printf("Process %d: c =", rank);
        for (int i = 0; i < N; i++)
            printf(" %d", result[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
