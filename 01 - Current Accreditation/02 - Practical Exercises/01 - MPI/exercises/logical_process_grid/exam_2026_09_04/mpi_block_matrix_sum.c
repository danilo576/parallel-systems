#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 7, 04.09.2026.
 * Napisati MPI program koji na osnovu matrica A i B dimenzija n x n
 * generise matricu C = alpha*A + B. Matrice A i B i skalar alpha
 * inicijalizuju se u master procesu. Ukupno p = q*q procesa uredjeno
 * je kao matrica q x q, pri cemu je n deljivo sa q.
 * Master distribuira blokove (n/q) x (n/q): proces P(r*q+s) dobija
 * vrste r*(n/q)..(r+1)*(n/q)-1 i kolone s*(n/q)..(s+1)*(n/q)-1.
 * Nakon distribucije svaki proces obavlja lokalno izracunavanje
 * i ucestvuje u generisanju konacnog rezultata.
 * Delove matrica slati direktno iz A i B point-to-point operacijama.
 * Ostatak zadatka implementirati koriscenjem grupnih operacija.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N][N], c[N][N], alpha;
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type, column_strip_type, resized_column_strip_type;

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
    double local_c_block[k][k], c_column_strip[N][k];

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);
    MPI_Type_vector(k, k, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    /* Place adjacent N-by-k column strips side by side in C. */
    MPI_Type_vector(N, k, N, MPI_DOUBLE, &column_strip_type);
    MPI_Type_create_resized(column_strip_type, 0, k * sizeof(double),
                            &resized_column_strip_type);
    MPI_Type_commit(&resized_column_strip_type);

    if (rank == 0)
    {
        alpha = 2.0;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                a[i][j] = i * N + j + 1;
                b[i][j] = i + j + 1;
            }
        printf("Alpha: %g\n\n", alpha);
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

    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MCW);

    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
            local_c_block[i][j] = alpha * local_a_block[i][j] + local_b_block[i][j];

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
