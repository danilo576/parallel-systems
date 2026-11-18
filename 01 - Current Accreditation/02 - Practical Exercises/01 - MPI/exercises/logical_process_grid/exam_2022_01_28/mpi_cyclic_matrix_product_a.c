#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef N
#define N 4
#endif

#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2a, 28.01.2022.
 * Napisati MPI program za množenje kvadratnih matrica A i B reda n.
 * Master inicijalizuje A i B. Procesi čine mrežu q × q, gde je q² = p.
 * Svi procesi u vrsti r dobijaju vrste r, r+q, ..., r+n−q matrice A.
 * Svi procesi u koloni c dobijaju kolone c, c+q, ..., c+n−q matrice B.
 * Svaki blok jedne matrice prenosi se odjednom. Procesi računaju svoje
 * blokove proizvoda C, koji se prikazuje u master procesu.
 * Raspodelu i sklapanje rezultata realizovati grupnim operacijama
 * i korišćenjem novih komunikatora.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int q = 1;
    while ((long long)q * q < size)
        q++;
    if ((long long)q * q != size || N <= 0 || N % q != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected p = q*q and positive N divisible by q.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int n = N;
    const int s = n / q;
    const int row = rank / q;
    const int col = rank % q;
    double a[N][N], b[N][N], c[N][N];
    double local_a[s][N], local_b[N][s], local_c[s][s];

    if (rank == 0)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
            {
                a[i][j] = i + 1;
                b[i][j] = j + 1;
            }

    MPI_Datatype a_strip, b_strip;
    MPI_Type_vector(s, n, q * n, MPI_DOUBLE, &a_strip);
    MPI_Type_vector(n * s, 1, q, MPI_DOUBLE, &b_strip);

    MPI_Comm row_comm, col_comm;
    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);

    MPI_Datatype a_scatter, b_scatter;
    MPI_Type_create_resized(a_strip, 0, n * sizeof(double), &a_scatter);
    MPI_Type_create_resized(b_strip, 0, sizeof(double), &b_scatter);
    MPI_Type_commit(&a_scatter);
    MPI_Type_commit(&b_scatter);

    if (col == 0)
        MPI_Scatter(a, 1, a_scatter, local_a, s * n, MPI_DOUBLE, 0, col_comm);
    MPI_Bcast(local_a, s * n, MPI_DOUBLE, 0, row_comm);

    if (row == 0)
        MPI_Scatter(b, 1, b_scatter, local_b, n * s, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(local_b, n * s, MPI_DOUBLE, 0, col_comm);

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++)
        {
            local_c[i][j] = 0;
            for (int t = 0; t < n; t++)
                local_c[i][j] += local_a[i][t] * local_b[t][j];
        }

    /* Interleave the rows of the local blocks in each process column. */
    double c_strip[N][s];
    MPI_Datatype c_rows, c_rows_resized;
    MPI_Type_vector(s, s, q * s, MPI_DOUBLE, &c_rows);
    MPI_Type_create_resized(c_rows, 0, s * sizeof(double), &c_rows_resized);
    MPI_Type_commit(&c_rows_resized);
    MPI_Gather(local_c, s * s, MPI_DOUBLE,
               c_strip, 1, c_rows_resized, 0, col_comm);

    /* Interleave the columns of the completed strips at P0. */
    if (row == 0)
        MPI_Gather(c_strip, n * s, MPI_DOUBLE, c, 1, b_scatter, 0, row_comm);

    if (rank == 0)
    {
        puts("Matrix C");
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                printf("%g%c", c[i][j], j == n - 1 ? '\n' : ' ');
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
