#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2, 26.03.2019. Master inicijalizuje matricu A dimenzija m × n
 * i vektor b dužine n. Broj kolona n deljiv je brojem procesa p.
 * Proces P_i dobija kolone A i elemente b sa indeksima l za koje važi
 * l mod p = i. Sve dodeljene kolone, odnosno svi dodeljeni elementi
 * vektora, šalju se odjednom. Svaki proces računa svoj doprinos
 * rezultatu c = A b. Zadatak rešiti grupnim operacijama.
 */
int main(int argc, char *argv[])
{
    int rank, process_count, dimensions[2];
    int *matrix = NULL, *vector = NULL, *result = NULL;
    MPI_Datatype matrix_stride, vector_stride, matrix_block, vector_block;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    if (rank == 0)
        scanf("%d%d", &dimensions[0], &dimensions[1]);
    MPI_Bcast(dimensions, 2, MPI_INT, 0, MCW);

    int rows = dimensions[0];
    int columns = dimensions[1];
    if (rows <= 0 || columns <= 0 || columns % process_count != 0) {
        if (rank == 0)
            fprintf(stderr, "Dimensions must be positive and columns divisible by process count.\n");
        MPI_Finalize();
        return 1;
    }

    int local_columns = columns / process_count;
    int local_count = rows * local_columns;
    int *local_matrix = malloc((size_t)local_count * sizeof *local_matrix);
    int *local_vector = malloc((size_t)local_columns * sizeof *local_vector);
    int *partial = calloc((size_t)rows, sizeof *partial);

    if (rank == 0) {
        size_t matrix_count = (size_t)rows * columns;
        matrix = malloc(matrix_count * sizeof *matrix);
        vector = malloc((size_t)columns * sizeof *vector);
        result = malloc((size_t)rows * sizeof *result);

        for (size_t index = 0; index < matrix_count; index++)
            scanf("%d", &matrix[index]);
        for (int column = 0; column < columns; column++)
            scanf("%d", &vector[column]);
    }

    MPI_Type_vector(local_count, 1, process_count, MPI_INT, &matrix_stride);
    MPI_Type_create_resized(matrix_stride, 0, sizeof(int), &matrix_block);
    MPI_Type_commit(&matrix_block);

    MPI_Type_vector(local_columns, 1, process_count, MPI_INT, &vector_stride);
    MPI_Type_create_resized(vector_stride, 0, sizeof(int), &vector_block);
    MPI_Type_commit(&vector_block);

    MPI_Scatter(matrix, 1, matrix_block, local_matrix, local_count, MPI_INT, 0, MCW);
    MPI_Scatter(vector, 1, vector_block, local_vector, local_columns, MPI_INT, 0, MCW);

    for (int row = 0; row < rows; row++)
        for (int column = 0; column < local_columns; column++)
            partial[row] += local_matrix[row * local_columns + column] * local_vector[column];

    MPI_Reduce(partial, result, rows, MPI_INT, MPI_SUM, 0, MCW);

    if (rank == 0) {
        printf("Result:");
        for (int row = 0; row < rows; row++)
            printf(" %d", result[row]);
        printf("\n");
    }

    free(matrix);
    free(vector);
    free(result);
    free(local_matrix);
    free(local_vector);
    free(partial);
    MPI_Finalize();
    return 0;
}
