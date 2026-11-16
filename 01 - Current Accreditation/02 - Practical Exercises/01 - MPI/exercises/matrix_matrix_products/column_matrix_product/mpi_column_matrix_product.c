#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define M 3
#define K 4
#define N 6

#define MCW MPI_COMM_WORLD

/*
 * 6. a) Napisati MPI program koji realizuje mnozenje matrice A dimenzija
 * m x k i matrice B dimenzija k x n, cime se dobija rezultujuca matrica C.
 * Mnozenje se obavlja tako sto master proces salje svakom procesu celu
 * matricu A i po n/p kolona matrice B (p - broj procesa, n deljivo sa p).
 * Elementi svih kolona matrice B koji se salju svakom procesu salju se
 * odjednom direktno iz matrice B. Svi procesi ucestvuju u izracunavanjima
 * potrebnim za generisanje rezultata programa. Zadatak realizovati
 * koriscenjem grupnih operacija, osim za slanje kolona matrice B za koje
 * se koriste Point-to-Point operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int local_cols;
    double a[M][K], b[K][N], c[M][N];
    double *local_b, *local_c;
    MPI_Datatype col_type;
    MPI_Datatype new_c;
    MPI_Datatype new_c_r;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (N % size != 0)
    {
        if (rank == 0)
            printf("N (%d) must be divisible by the number of processes (%d).\n", N, size);

        MPI_Finalize();
        return EXIT_FAILURE;
    }

    local_cols = N / size;

    local_b = malloc((size_t)K * local_cols * sizeof *local_b);
    local_c = calloc((size_t)M * local_cols, sizeof *local_c);

    if (local_b == NULL || local_c == NULL)
    {
        fprintf(stderr, "Process %d: memory allocation failed.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }
    
    MPI_Type_vector(K, local_cols, N, MPI_DOUBLE, &col_type);
    MPI_Type_commit(&col_type);

    MPI_Type_vector(M, local_cols, N, MPI_DOUBLE, &new_c);
    MPI_Type_commit(&new_c);
    MPI_Type_create_resized(new_c, 0, local_cols * sizeof(double), &new_c_r);
    MPI_Type_commit(&new_c_r);

    if (rank == 0)
    {
        for (int i = 0; i < M; i++)
            for (int j = 0; j < K; j++)
                a[i][j] = i * K + j;

        for (int i = 0; i < K; i++)
            for (int j = 0; j < N; j++)
                b[i][j] = i * N + j;

        print_matrix("Initial matrix A", M, K, &a[0][0]);
        print_matrix("Initial matrix B", K, N, &b[0][0]);
    }
    
    if (rank == 0)
    {
        for (int i = 0; i < K; i++)
            for (int j = 0; j < local_cols; j++)
                local_b[i * local_cols + j] = b[i][j];

        for (int p = 1; p < size; p++)
            MPI_Send(&b[0][p * local_cols], 1, col_type, p, 0, MCW);
    }
    else
    {
        MPI_Recv(local_b, K * local_cols, MPI_DOUBLE, 0, 0, MCW, MPI_STATUS_IGNORE);
    }

    MPI_Bcast(a, M * K, MPI_DOUBLE, 0, MCW);

    print_distribution(rank, size,
        "Matrix A", M, K, &a[0][0],
        "Local B", K, local_cols, local_b);

    for (int i = 0; i < M; i++)
        for (int j = 0; j < local_cols; j++)
            for (int q = 0; q < K; q++)
                local_c[i * local_cols + j] += a[i][q] * local_b[q * local_cols + j];

    MPI_Gather(local_c, M * local_cols, MPI_DOUBLE, &c[0][0], 1, new_c_r, 0, MCW);

    if (rank == 0)
       print_matrix("Matrix C", M, N, &c[0][0]);
    
    free(local_b);
    free(local_c);

    MPI_Finalize();

    return 0;

}
