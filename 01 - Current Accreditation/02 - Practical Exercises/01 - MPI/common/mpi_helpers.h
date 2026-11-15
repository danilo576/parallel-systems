#ifndef MPI_HELPERS_H
#define MPI_HELPERS_H

#include <stdio.h>
#include <mpi.h>

/* Matrices use contiguous row-major storage. */
static inline void print_matrix(const char *label, int rows, int cols,
                                const double *matrix)
{
    printf("%s (%d x %d):\n", label, rows, cols);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
            printf("%8g ", matrix[i * cols + j]);
        printf("\n");
    }
    printf("\n");
    fflush(stdout);
}

static inline void print_process_distribution(
    int rank,
    const char *a_label, int a_rows, int a_cols, const double *local_a,
    const char *b_label, int b_rows, int b_cols, const double *local_b)
{
    const char *colors[] = {
        "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m"
    };

    printf("%s=== Process %d ===\n", colors[rank % 6], rank);
    print_matrix(a_label, a_rows, a_cols, local_a);
    print_matrix(b_label, b_rows, b_cols, local_b);
    printf("=== End process %d ===\033[0m\n\n", rank);
    fflush(stdout);
}

/* All processes call this function with the same local dimensions.
 * Each local matrix must be a contiguous array of doubles in row-major order.
 * This helper gathers copies for display only; it does not distribute data.
 */
static inline void print_distribution(
    int rank, int size,
    const char *a_label, int a_rows, int a_cols, const double *local_a,
    const char *b_label, int b_rows, int b_cols, const double *local_b)
{
    int a_count = a_rows * a_cols;
    int b_count = b_rows * b_cols;
    double received_a[a_count * size];
    double received_b[b_count * size];

    MPI_Gather(local_a, a_count, MPI_DOUBLE,
               received_a, a_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(local_b, b_count, MPI_DOUBLE,
               received_b, b_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
        for (int process = 0; process < size; process++)
            print_process_distribution(
                process,
                a_label, a_rows, a_cols, received_a + process * a_count,
                b_label, b_rows, b_cols, received_b + process * b_count);
}

#endif
