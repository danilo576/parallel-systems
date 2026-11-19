#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../../common/mpi_helpers.h"

#define MCW MPI_COMM_WORLD

/*
 * 09.10.2025, zadatak 7a.
 * Pomnožiti matrice A i B dimenzija n x n i prikazati rezultat C
 * u master procesu, koji inicijalizuje obe matrice. U mreži q x q
 * važi p = q*q i q = n. Proces (i, j) treba da dobije celu i-tu
 * vrstu A i celu j-tu kolonu B. Delove svake matrice slati odjednom,
 * direktno iz matrice. Za komunikaciju koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int n = (int)sqrt(size);
    if (n * n != size) {
        if (rank == 0)
            fprintf(stderr, "Use a square process grid.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int row = rank / n;
    int col = rank % n;
    MPI_Comm row_comm, col_comm;

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);

    double *a = NULL, *b = NULL, *c = NULL;
    if (rank == 0) {
        a = malloc(size * sizeof(*a));
        b = malloc(size * sizeof(*b));
        c = malloc(size * sizeof(*c));
        for (int i = 0; i < size; i++) {
            a[i] = i + 1;
            b[i] = i + 1;
        }
    }

    double local_a[n], local_b[n];

    if (col == 0)
        MPI_Scatter(a, n, MPI_DOUBLE, local_a, n, MPI_DOUBLE, 0, col_comm);
    MPI_Bcast(local_a, n, MPI_DOUBLE, 0, row_comm);

    if (row == 0) {
        MPI_Datatype column, packed_column;

        MPI_Type_vector(n, 1, n, MPI_DOUBLE, &column);
        MPI_Type_create_resized(column, 0, sizeof(double), &packed_column);
        MPI_Type_commit(&packed_column);
        MPI_Scatter(b, 1, packed_column, local_b, n, MPI_DOUBLE, 0, row_comm);
    }
    MPI_Bcast(local_b, n, MPI_DOUBLE, 0, col_comm);

    double local_c = 0;
    for (int k = 0; k < n; k++)
        local_c += local_a[k] * local_b[k];

    MPI_Gather(&local_c, 1, MPI_DOUBLE, c, 1, MPI_DOUBLE, 0, MCW);

    if (rank == 0) {
        print_matrix("Matrix A", n, n, a);
        print_matrix("Matrix B", n, n, b);
        print_matrix("Matrix C", n, n, c);
        free(a);
        free(b);
        free(c);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
