#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * 1. zadatak, 02.10.2023.
 * Napisati MPI program koji realizuje mnozenje matrice A dimenzija n x n
 * i vektora b duzine n. Matrica A i vektor b inicijalizuju se u master
 * procesu. Broj procesa je p i uredjeni su kao matrica q x q (q*q = p).
 * Master distribuira blokove matrice A kao na slici za n = 8 i p = 16:
 * proces Pi dobija vrste i/q, i/q+q, ..., i/q+n-q i kolone
 * i%q, i%q+q, ..., i%q+n-q (celobrojno deljenje i/q).
 * Proces Pi dobija elemente vektora b sa indeksima i%q, i%q+q,
 * i%q+2q, ..., i%q+n-q. Svaki proces, ukljucujuci master, obavlja
 * odgovarajuca izracunavanja i ucestvuje u generisanju vektora c.
 * Vektor c treba upisati u binarni fajl "okt2-c.dat", tako da u upisu
 * ucestvuje vise procesa. Slanje vrednosti bloka matrice A svakom
 * procesu obaviti odjednom. Slanje blokova A i vektora b, kao i
 * generisanje rezultata implementirati grupnim operacijama i
 * funkcijama za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type, vector_type, vector_resized;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    while (q < size / q)
        q++;
    if (q * q != size || q < 2 || N <= 0 || N % q != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected p = q*q >= 4 and positive N divisible by q.\n");
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

        print_matrix("Initial matrix A", N, N, &a[0][0]);
        print_matrix("Initial vector B", N, 1, b);
    }

    MPI_Type_vector(rows * cols, 1, q, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    double row_a[rows][N];
    MPI_Datatype block_resized;
    MPI_Type_create_resized(block_type, 0, sizeof(double), &block_resized);
    MPI_Type_commit(&block_resized);

    MPI_Datatype rows_type, rows_resized;
    MPI_Type_vector(rows, N, q * N, MPI_DOUBLE, &rows_type);
    MPI_Type_create_resized(rows_type, 0, N * sizeof(double), &rows_resized);
    MPI_Type_commit(&rows_resized);

    if (col == 0)
        MPI_Scatter(a, 1, rows_resized, row_a, rows * N,
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

    for (int i = 0; i < rows; i++)
    {
        local_c[i] = 0;
        for (int j = 0; j < cols; j++)
            local_c[i] += local_a[i][j] * local_b[j];
    }

    MPI_Reduce(local_c, row_c, rows, MPI_DOUBLE, MPI_SUM, 0, row_comm);

    if (col == 0)
    {
        MPI_File file;
        MPI_Offset offset = (MPI_Offset)row * sizeof(double);
        MPI_Datatype file_type;
        MPI_Type_vector(rows, 1, q, MPI_DOUBLE, &file_type);
        MPI_Type_commit(&file_type);

        MPI_File_open(col_comm, "okt2-c.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                      MPI_INFO_NULL, &file);
        MPI_File_set_size(file, (MPI_Offset)N * sizeof(double));
        MPI_File_set_view(file, offset, MPI_DOUBLE, file_type, "native",
                          MPI_INFO_NULL);
        MPI_File_write_all(file, row_c, rows, MPI_DOUBLE, MPI_STATUS_IGNORE);
        MPI_File_close(&file);
    }

    if (rank == 0)
        printf("Vector C written to okt2-c.dat (%d doubles).\n", N);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
