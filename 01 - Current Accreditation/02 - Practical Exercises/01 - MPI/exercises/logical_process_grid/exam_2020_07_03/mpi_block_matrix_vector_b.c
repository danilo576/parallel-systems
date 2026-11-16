#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * 03.07.2020.
 * Napisati MPI program za mnozenje matrice A dimenzija n x n i vektora b
 * duzine n, cime se dobija vektor c duzine n. A i b inicijalizuje master.
 * Procesi su rasporedjeni u mrezu q x q, p = q*q. Matrica A je podeljena
 * na blokove: proces Pi dobija n/q uzastopnih vrsta i kolone i%q,
 * i%q+q, ..., i%q+n-q. Iste indekse dobija iz vektora b.
 * Svi procesi, ukljucujuci master, ucestvuju u racunanju. Rezultat se
 * prikazuje u procesu koji sadrzi minimum matrice A. Slanje vektora b
 * i generisanje rezultata realizovati grupnim operacijama i novim
 * komunikatorima. Pretpostaviti da je n deljivo sa q.
 * b) Slanje blokova matrice A realizovati grupnim operacijama
 * i funkcijama za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N], c[N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type, vector_type, vector_resized;
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

    MPI_Type_vector(rows * cols, 1, q, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    double row_a[rows][N];
    MPI_Datatype block_resized;
    MPI_Type_create_resized(block_type, 0, sizeof(double), &block_resized);
    MPI_Type_commit(&block_resized);

    if (col == 0)
        MPI_Scatter(a, rows * N, MPI_DOUBLE, row_a, rows * N,
                    MPI_DOUBLE, 0, col_comm);
    MPI_Scatter(row_a, 1, block_resized, local_a, rows * cols,
                MPI_DOUBLE, 0, row_comm);

    MPI_Type_vector(cols, 1, q, MPI_DOUBLE, &vector_type);
    MPI_Type_create_resized(vector_type, 0, sizeof(double), &vector_resized);
    MPI_Type_commit(&vector_resized);

    if (row == 0)
        MPI_Scatter(b, 1, vector_resized, local_b, cols, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(local_b, cols, MPI_DOUBLE, 0, col_comm);

    print_distribution(rank, size,
                       "Local matrix A", rows, cols, &local_a[0][0],
                       "Local vector B", 1, cols, local_b);

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
    if (col == result_col)
        MPI_Gather(row_c, rows, MPI_DOUBLE, c, rows, MPI_DOUBLE, result_row, col_comm);

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
