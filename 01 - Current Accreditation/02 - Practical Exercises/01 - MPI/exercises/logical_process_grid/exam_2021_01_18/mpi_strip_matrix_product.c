#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 3, 18.01.2021.
 * Napisati MPI program koji mnozi matrice A i B dimenzija n x n i dobija
 * matricu C dimenzija n x n. Matrice A i B inicijalizuje master proces.
 * Broj procesa je p, a procesi su uredjeni kao matrica q x q (q*q = p).
 * Matrica A je podeljena na blokove od n/q vrsta, a B na blokove od
 * n/q kolona. Master distribuira odgovarajuce blokove svim procesima:
 * procesi u istoj vrsti mreze dobijaju isti blok vrsta A, a procesi u
 * istoj koloni mreze isti blok kolona B. Svaki proces obavlja odgovarajuca
 * izracunavanja i ucestvuje u generisanju rezultata koji prikazuje master.
 * Za slanje blokova A i B koristiti izvedene tipove podataka.
 * Slanje blokova i generisanje rezultata implementirati grupnim
 * operacijama i funkcijama za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N][N], c[N][N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype row_strip_type, column_strip_type, resized_column_strip_type;

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

    int row = rank / q, col = rank % q, k = N / q;
    double a_row_strip[k][N], b_column_strip[N][k];
    double local_c_block[k][k], c_column_strip[N][k];

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);
    MPI_Type_contiguous(k * N, MPI_DOUBLE, &row_strip_type);
    MPI_Type_commit(&row_strip_type);

    /* Describe N-by-k strips in B and C, with adjacent strip origins. */
    MPI_Type_vector(N, k, N, MPI_DOUBLE, &column_strip_type);
    MPI_Type_create_resized(column_strip_type, 0, k * sizeof(double),
                            &resized_column_strip_type);
    MPI_Type_commit(&resized_column_strip_type);

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                a[i][j] = i * N + j + 1;
                b[i][j] = i + j + 1;
            }
        print_matrix("Matrix A", N, N, &a[0][0]);
        print_matrix("Matrix B", N, N, &b[0][0]);
    }

    /* Distribute A through column 0, then replicate each strip along its row. */
    if (col == 0)
        MPI_Scatter(a, 1, row_strip_type,
                    a_row_strip, k * N, MPI_DOUBLE, 0, col_comm);
    MPI_Bcast(a_row_strip, k * N, MPI_DOUBLE, 0, row_comm);

    /* Distribute B through row 0, then replicate each strip along its column. */
    if (row == 0)
        MPI_Scatter(b, 1, resized_column_strip_type,
                    b_column_strip, N * k, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(b_column_strip, N * k, MPI_DOUBLE, 0, col_comm);

    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
        {
            local_c_block[i][j] = 0;
            for (int t = 0; t < N; t++)
                local_c_block[i][j] += a_row_strip[i][t] * b_column_strip[t][j];
        }

    /* Gather each column of blocks into an N-by-k strip in row 0. */
    MPI_Gather(local_c_block, k * k, MPI_DOUBLE,
               c_column_strip, k * k, MPI_DOUBLE, 0, col_comm);

    /* Only row 0 owns complete column strips. */
    if (row == 0)
        MPI_Gather(c_column_strip, N * k, MPI_DOUBLE,
                   c, 1, resized_column_strip_type, 0, row_comm);

    if (rank == 0)
        print_matrix("Matrix C", N, N, &c[0][0]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
