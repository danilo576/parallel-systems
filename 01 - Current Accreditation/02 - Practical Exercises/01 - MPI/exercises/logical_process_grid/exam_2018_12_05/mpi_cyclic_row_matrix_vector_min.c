#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 4
#define ROOT 0

/*
 * 05.12.2018, zadatak 2a
 *
 * Napisati MPI program koji množi matricu A dimenzija n × n vektorom b
 * dužine n i dobija vektor c. Master proces inicijalizuje A i b.
 * Za p procesa, gde je n deljivo sa p, proces P_i dobija vrste matrice A
 * sa indeksima i, i+p, i+2p, ..., i+n-p. Master šalje svakom procesu
 * njegov blok vrsta odjednom, direktno iz matrice, i ceo vektor b.
 * Svi procesi učestvuju u računanju i formiranju rezultata. Vektor c
 * prikazati u procesu koji sadrži njegov najmanji element.
 * Koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int a[N][N], b[N], c[N];
    struct {
        int value;
        int rank;
    } local_min, global_min;
    MPI_Datatype row_stride, row_block, result_stride, result_block;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0) {
        if (rank == ROOT)
            fprintf(stderr, "N must be divisible by the number of processes.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int local_rows = N / size;
    int local_a[local_rows][N], local_c[local_rows];

    if (rank == ROOT) {
        for (int col = 0; col < N; col++)
            b[col] = 1;
        for (int row = 0; row < N; row++)
            for (int col = 0; col < N; col++)
                a[row][col] = row == 1 ? -1 : row + 1;
    }

    MPI_Type_vector(local_rows, N, size * N, MPI_INT, &row_stride);
    MPI_Type_create_resized(row_stride, 0, N * sizeof(int), &row_block);
    MPI_Type_commit(&row_block);

    MPI_Scatter(a, 1, row_block, local_a, local_rows * N, MPI_INT,
                ROOT, MPI_COMM_WORLD);
    MPI_Bcast(b, N, MPI_INT, ROOT, MPI_COMM_WORLD);

    for (int row = 0; row < local_rows; row++) {
        local_c[row] = 0;
        for (int col = 0; col < N; col++)
            local_c[row] += local_a[row][col] * b[col];
    }

    local_min.value = local_c[0];
    local_min.rank = rank;
    for (int row = 1; row < local_rows; row++)
        if (local_c[row] < local_min.value)
            local_min.value = local_c[row];

    MPI_Reduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC,
               ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&global_min.rank, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    MPI_Type_vector(local_rows, 1, size, MPI_INT, &result_stride);
    MPI_Type_create_resized(result_stride, 0, sizeof(int), &result_block);
    MPI_Type_commit(&result_block);

    MPI_Gather(local_c, local_rows, MPI_INT, c, 1, result_block,
               global_min.rank, MPI_COMM_WORLD);

    if (rank == global_min.rank) {
        printf("Result on process %d:\n", rank);
        for (int row = 0; row < N; row++)
            printf("c[%d] = %d\n", row, c[row]);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
