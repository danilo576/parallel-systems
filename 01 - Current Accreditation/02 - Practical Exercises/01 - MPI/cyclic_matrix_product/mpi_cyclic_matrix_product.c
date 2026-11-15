#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../mpi_helpers.h"

#define N 3
#define R 4
#define K 6

#define MCW MPI_COMM_WORLD  

/*
 * Napisati MPI program koji realizuje mnozenje matrica A dimenzija n x r
 * i B dimenzija r x k. Program takodje pronalazi proizvod elemenata svake
 * vrste matrice B.
 *
 * Matrice A i B se inicijalizuju u master procesu. Master salje svakom
 * procesu celu matricu A, dok je B podeljena u blokove po kolonama,
 * po k/p kolona, tako da proces Pi dobija kolone sa indeksima
 * i, i+p, ..., i+(k/p-1)*p, gde je p broj procesa.
 *
 * Slanje kolona se radi odjednom. Rezultat prikazati u procesu koji
 * sadrzi maksimalnu vrednost u matrici B.
 *
 * Zadatak resiti koriscenjem grupnih operacija.
 */
int main(int argc, char *argv[])
{
    int rank, size, local_cols;
    double a[N][R], b[R][K], c[N][K];
    double *local_b;
    double *local_c;
    double local_product_row_b[R], product_row_b[R];
    MPI_Datatype new_b, new_b_r;
    MPI_Datatype new_c, new_c_r;
    struct {
        double value;
        int rank;
    } local_max_b, global_max_b;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K % size != 0) 
    {
        if (rank == 0)
            printf("K (%d) must be divisible by the number of processes (%d).\n", K, size);

        MPI_Finalize();
        return EXIT_FAILURE;
    }

    local_cols = K / size;
    local_b = malloc((size_t)R * local_cols * sizeof *local_b);
    local_c = calloc((size_t)N * local_cols, sizeof *local_c);

    if (local_b == NULL || local_c == NULL)
    {
        fprintf(stderr, "Process %d: memory allocation failed.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }

    MPI_Type_vector(R * local_cols, 1, size, MPI_DOUBLE, &new_b);
    MPI_Type_commit(&new_b);
    MPI_Type_create_resized(new_b, 0, sizeof(double), &new_b_r);
    MPI_Type_commit(&new_b_r);

    MPI_Type_vector(N * local_cols, 1, size, MPI_DOUBLE, &new_c);
    MPI_Type_commit(&new_c);
    MPI_Type_create_resized(new_c, 0, sizeof(double), &new_c_r);
    MPI_Type_commit(&new_c_r);

    if (rank == 0) 
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < R; j++)
                a[i][j] = i + j + 1;
        
        for (int i = 0; i < R; i++)
            for (int j = 0; j < K; j++)
                b[i][j] = (i + j) % 3 + 1;

        print_matrix("Initial matrix A", N, R, &a[0][0]);
        print_matrix("Initial matrix B", R, K, &b[0][0]);
    }

    MPI_Bcast(a, N * R, MPI_DOUBLE, 0, MCW);
    MPI_Scatter(b, 1, new_b_r, local_b, R * local_cols, MPI_DOUBLE, 0, MCW);

    print_distribution(rank, size,
                       "Matrix A", N, R, &a[0][0],
                       "Local B", R, local_cols, local_b);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < local_cols; j++)
            for (int q = 0; q < R; q++)
                local_c[i * local_cols + j] += a[i][q] * local_b[q * local_cols + j];

    for (int i = 0; i < R; i++) 
    {
        local_product_row_b[i] = 1;
        for (int j = 0; j < local_cols; j++)
            local_product_row_b[i] *= local_b[i * local_cols + j];
    }

    local_max_b.value = local_b[0];
    local_max_b.rank = rank;

    for (int i = 1; i < R * local_cols; i++)
        if (local_b[i] > local_max_b.value)
            local_max_b.value = local_b[i];

    MPI_Reduce(&local_max_b, &global_max_b, 1, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MCW);
    MPI_Bcast(&global_max_b, 1, MPI_DOUBLE_INT, 0, MCW);
    MPI_Reduce(local_product_row_b, product_row_b, R, MPI_DOUBLE, MPI_PROD, global_max_b.rank, MCW);
    MPI_Gather(local_c, N * local_cols, MPI_DOUBLE, c, 1, new_c_r, global_max_b.rank, MCW);

    if (rank == global_max_b.rank)
    {
        print_matrix("Matrix C", N, K, &c[0][0]);

        printf("Row product B:\n");
        for (int i = 0; i < R; i++)
            printf("%g ", product_row_b[i]); 
        printf("\n");       
    }
        
    free(local_b);
    free(local_c);
    MPI_Finalize();

    return 0;
}