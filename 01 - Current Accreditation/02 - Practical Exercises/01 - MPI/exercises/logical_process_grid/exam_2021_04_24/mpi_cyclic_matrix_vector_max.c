#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * 3. zadatak, 24.04.2021.
 * Napisati MPI program za mnozenje matrice A dimenzija m x m i vektora b
 * duzine m, cime se dobija vektor c duzine m. A i b inicijalizuje master.
 * Broj procesa je p, uredjeni su kao matrica q x q, q*q = p.
 * Za m = 8 i p = 16 raspodela je kao na slici: proces Pi dobija vrste
 * i/q, i/q+q, ..., i/q+m-q i kolone i%q, i%q+q, ..., i%q+m-q.
 * Iz vektora b dobija indekse i%q, i%q+q, ..., i%q+m-q.
 * Svaki proces, ukljucujuci master, racuna i ucestvuje u generisanju
 * rezultata koji se prikazuje u procesu koji sadrzi maksimum matrice A.
 * Blok A svakom drugom procesu poslati jednom MPI_Send naredbom,
 * koristeci count = 1 i izvedeni tip. Slanje delova b i generisanje
 * rezultata realizovati grupnim operacijama i novim komunikatorima.
 * Pretpostaviti da je m deljivo sa q. U kodu je m oznaceno sa N.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N], c[N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype column_type, column_resized, block_type, vector_type, vector_resized;
    struct {
        double value;
        int rank;
    } local_max, global_max;

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

        print_matrix("Initial matrix A", N, N, &a[0][0]);
        print_matrix("Initial vector B", N, 1, b);
    }

    MPI_Type_vector(cols, 1, q, MPI_DOUBLE, &column_type);
    MPI_Type_create_resized(column_type, 0, N * sizeof(double), &column_resized);
    MPI_Type_vector(rows, 1, q, column_resized, &block_type);
    MPI_Type_commit(&block_type);

    if (rank == 0)
    {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                local_a[i][j] = a[i * q][j * q];
        for (int dest = 1; dest < size; dest++)
            MPI_Send(&a[dest / q][dest % q], 1, block_type,
                     dest, 0, MCW);
    }
    else
        MPI_Recv(local_a, rows * cols, MPI_DOUBLE, 0, 0,
                 MCW, MPI_STATUS_IGNORE);

    MPI_Type_vector(cols, 1, q, MPI_DOUBLE, &vector_type);
    MPI_Type_create_resized(vector_type, 0, sizeof(double), &vector_resized);
    MPI_Type_commit(&vector_resized);

    if (row == 0)
        MPI_Scatter(b, 1, vector_resized, local_b, cols, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(local_b, cols, MPI_DOUBLE, 0, col_comm);

    print_distribution(rank, size,
                       "Local matrix A", rows, cols, &local_a[0][0],
                       "Local vector B", 1, cols, local_b);

    local_max.value = local_a[0][0];
    local_max.rank = rank;
    for (int i = 0; i < rows; i++)
    {
        local_c[i] = 0;
        for (int j = 0; j < cols; j++)
        {
            local_c[i] += local_a[i][j] * local_b[j];
            if (local_a[i][j] > local_max.value)
                local_max.value = local_a[i][j];
        }
    }

    MPI_Reduce(&local_max, &global_max, 1, MPI_DOUBLE_INT,
               MPI_MAXLOC, 0, MCW);
    MPI_Bcast(&global_max, 1, MPI_DOUBLE_INT, 0, MCW);
    int result_row = global_max.rank / q;
    int result_col = global_max.rank % q;

    MPI_Reduce(local_c, row_c, rows, MPI_DOUBLE, MPI_SUM, result_col, row_comm);
    if (col == result_col)
        MPI_Gather(row_c, rows, MPI_DOUBLE, c, 1, vector_resized,
                   result_row, col_comm);

    if (rank == global_max.rank)
    {
        printf("Result process: %d\nMaximum A: %g\nVector C:", rank, global_max.value);
        for (int i = 0; i < N; i++)
            printf(" %g", c[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
