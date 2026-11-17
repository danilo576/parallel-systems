#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2, 07.10.2021.
 * Napisati MPI program koji mnozi matricu A dimenzija k x k i vektor b
 * duzine k, cime se dobija vektor c. A i b inicijalizuje master proces.
 * Broj procesa je p = q*q i uredjeni su kao matrica q x q.
 * Za s = k/q, proces P(r*q+t) dobija vrste r, r+q, ..., r+(s-1)*q
 * i kolone t*s, ..., t*s+s-1 matrice A, kao na slici za k=8 i p=16.
 * Proces Pi dobija elemente b sa indeksima (i%q)*s do (i%q)*s+s-1.
 * Slanje vrednosti bloka A svakom procesu obaviti odjednom.
 * Svaki proces, ukljucujuci master, racuna i ucestvuje u generisanju
 * rezultata. Rezultat prikazati u procesu koji sadrzi minimum matrice A.
 * Slanje blokova A i vektora b, kao i generisanje rezultata, realizovati
 * grupnim operacijama i funkcijama za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N], c[N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type, block_resized, result_type, result_resized;
    struct {
        double value;
        int rank;
    } local_min, global_min;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    while (q < size / q)
        q++;
    if (q * q != size || N <= 0 || N % q != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected p = q*q and positive N divisible by q.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int row = rank / q, col = rank % q;
    int rows = N / q, cols = N / q;
    double local_a[rows][cols], local_b[cols];
    double local_c[rows], row_c[rows];

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);

    if (rank == 0)
    {
        for (int j = 0; j < N; j++)
            b[j] = j + 1;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                a[i][j] = i * N + j + 1;
        a[N - 1][N - 1] = -1;

        print_matrix("Initial matrix A", N, N, &a[0][0]);
        print_matrix("Initial vector B", N, 1, b);
    }

    MPI_Type_vector(rows, cols, q * N, MPI_DOUBLE, &block_type);
    MPI_Type_create_resized(block_type, 0, cols * sizeof(double), &block_resized);
    MPI_Type_commit(&block_resized);

    MPI_Scatter(a, 1, block_resized, local_a, rows * cols,
                MPI_DOUBLE, 0, MCW);

    if (row == 0)
        MPI_Scatter(b, cols, MPI_DOUBLE, local_b, cols, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(local_b, cols, MPI_DOUBLE, 0, col_comm);

    local_min.value = local_a[0][0];
    local_min.rank = rank;
    for (int i = 0; i < rows; i++)
    {
        local_c[i] = 0;
        for (int j = 0; j < cols; j++)
        {
            local_c[i] += local_a[i][j] * local_b[j];
            if (local_a[i][j] < local_min.value)
                local_min.value = local_a[i][j];
        }
    }

    MPI_Reduce(&local_min, &global_min, 1, MPI_DOUBLE_INT,
               MPI_MINLOC, 0, MCW);
    MPI_Bcast(&global_min, 1, MPI_DOUBLE_INT, 0, MCW);
    int result_row = global_min.rank / q;
    int result_col = global_min.rank % q;

    MPI_Reduce(local_c, row_c, rows, MPI_DOUBLE, MPI_SUM, result_col, row_comm);
    MPI_Type_vector(rows, 1, q, MPI_DOUBLE, &result_type);
    MPI_Type_create_resized(result_type, 0, sizeof(double), &result_resized);
    MPI_Type_commit(&result_resized);
    if (col == result_col)
        MPI_Gather(row_c, rows, MPI_DOUBLE, c, 1, result_resized, result_row, col_comm);

    if (rank == global_min.rank)
    {
        printf("Result process: %d\nMinimum A: %g\nVector C:", rank, global_min.value);
        for (int i = 0; i < N; i++)
            printf(" %g", c[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
