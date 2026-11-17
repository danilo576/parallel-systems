#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#ifndef N
#define N 8
#endif
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2, 23.06.2022.
 * Napisati MPI program koji mnozi matrice A i B dimenzija n x n i dobija
 * matricu C dimenzija n x n. Matrice A i B inicijalizuje master proces.
 * Broj procesa je p, a procesi su uredjeni kao matrica q x q (q*q = p).
 * Matrice su podeljene na blokove k x k, gde je k = n/q. Proces P(r*q+s)
 * dobija blok sa vrstama r*k do (r+1)*k-1 i kolonama s*k do (s+1)*k-1.
 * Master distribuira odgovarajuce blokove A i B, pri cemu se svih k*k
 * elemenata jednog bloka salje odjednom. Samo za pocetnu distribuciju
 * dozvoljene su point-to-point operacije.
 * Nakon distribucije grupnim operacijama obezbediti da svaki proces
 * sadrzi sve blokove A iz svoje vrste procesa i sve blokove B iz svoje
 * kolone procesa. Svaki proces obavlja odgovarajuca izracunavanja, a
 * rezultat se prikazuje u master procesu. Za slanje blokova A i B posle
 * pocetne distribucije i generisanje rezultata koristiti iskljucivo
 * grupne operacije i funkcije za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N][N], c[N][N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type, row_strip_block_type, column_strip_type, resized_column_strip_type;

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
    double local_a_block[k][k], local_b_block[k][k];
    double a_row_strip[k][N], b_column_strip[N][k];
    double local_c_block[k][k], c_column_strip[N][k];

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);
    MPI_Type_vector(k, k, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    /* Place adjacent blocks side by side in a k-by-N row strip. */
    MPI_Type_create_resized(block_type, 0, k * sizeof(double), &row_strip_block_type);
    MPI_Type_commit(&row_strip_block_type);

    /* Place adjacent N-by-k column strips side by side in C. */
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

        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
            {
                local_a_block[i][j] = a[i][j];
                local_b_block[i][j] = b[i][j];
            }
        for (int dest = 1; dest < size; dest++)
        {
            int first_row = (dest / q) * k;
            int first_col = (dest % q) * k;
            MPI_Send(&a[first_row][first_col], 1, block_type, dest, 0, MCW);
            MPI_Send(&b[first_row][first_col], 1, block_type, dest, 1, MCW);
        }
    }
    else
    {
        MPI_Recv(local_a_block, k * k, MPI_DOUBLE, 0, 0, MCW, MPI_STATUS_IGNORE);
        MPI_Recv(local_b_block, k * k, MPI_DOUBLE, 0, 1, MCW, MPI_STATUS_IGNORE);
    }

    MPI_Gather(local_a_block, k * k, MPI_DOUBLE,
               a_row_strip, 1, row_strip_block_type, 0, row_comm);
    MPI_Bcast(a_row_strip, k * N, MPI_DOUBLE, 0, row_comm);

    MPI_Gather(local_b_block, k * k, MPI_DOUBLE,
               b_column_strip, k * k, MPI_DOUBLE, 0, col_comm);
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
