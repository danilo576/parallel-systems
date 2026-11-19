#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define M 4
#define K 4

/*
 * 12.09.2025, zadatak 1a
 *
 * Napisati MPI program koji množi matricu A dimenzija m × k vektorom b
 * dužine k i računa proizvod elemenata svake kolone matrice A.
 * Master inicijalizuje A i b. Za p procesa, gde je k deljivo sa p,
 * proces P_i dobija vrste i, i+p, i+2p, ..., i+m-p iz matrice A.
 * Sve vrste za jedan proces šalju se odjednom, direktno iz matrice.
 * Svaki proces odjednom dobija ceo b, obavlja lokalna izračunavanja
 * i učestvuje u formiranju rezultata. Koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int a[M][K], b[K], c[M], products[K], local_products[K];
    MPI_Datatype row_stride, row_block, result_stride, result_block;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (M % size != 0 || K % size != 0) {
        if (rank == 0)
            fprintf(stderr, "M and K must be divisible by the number of processes.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int local_rows = M / size;
    int local_a[local_rows][K], local_c[local_rows];

    if (rank == 0) {
        for (int row = 0; row < M; row++)
            for (int col = 0; col < K; col++)
                a[row][col] = row * K + col + 1;

        for (int col = 0; col < K; col++)
            b[col] = 1;
    }

    MPI_Type_vector(local_rows, K, size * K, MPI_INT, &row_stride);
    MPI_Type_create_resized(row_stride, 0, K * sizeof(int), &row_block);
    MPI_Type_commit(&row_block);
    MPI_Scatter(a, 1, row_block, local_a, local_rows * K, MPI_INT, 0, MCW);
    MPI_Bcast(b, K, MPI_INT, 0, MCW);

    for (int col = 0; col < K; col++)
        local_products[col] = 1;

    for (int row = 0; row < local_rows; row++) {
        local_c[row] = 0;
        for (int col = 0; col < K; col++) {
            local_c[row] += local_a[row][col] * b[col];
            local_products[col] *= local_a[row][col];
        }
    }

    MPI_Type_vector(local_rows, 1, size, MPI_INT, &result_stride);
    MPI_Type_create_resized(result_stride, 0, sizeof(int), &result_block);
    MPI_Type_commit(&result_block);
    MPI_Gather(local_c, local_rows, MPI_INT, c, 1, result_block, 0, MCW);
    MPI_Reduce(local_products, products, K, MPI_INT, MPI_PROD, 0, MCW);

    if (rank == 0) {
        printf("A * b:");
        for (int row = 0; row < M; row++)
            printf(" %d", c[row]);
        printf("\nColumn products:");
        for (int col = 0; col < K; col++)
            printf(" %d", products[col]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
