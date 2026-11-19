#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Zadatak 1, 26.01.2024. Napisati MPI program za množenje matrice A
 * dimenzija k × n i vektora b dužine n. Master inicijalizuje A i b.
 * Proces P_i dobija sledećih 2^i uzastopnih vrsta matrice A jednim
 * pozivom MPI_Send, a ceo vektor b se šalje svim procesima grupnom
 * operacijom. Svaki proces računa svoj deo rezultata i učestvuje u
 * sklapanju vektora d, koji treba da se nađe u procesu sa najviše
 * vrsta. Dati primer A i b, ilustrovati raspodelu i rezultat i navesti
 * komandu za pokretanje. Pretpostaviti odgovarajuće dimenzije.
 */
int main(int argc, char *argv[])
{
    int rank, process_count, dimensions[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    if (rank == 0)
        scanf("%d%d", &dimensions[0], &dimensions[1]);
    MPI_Bcast(dimensions, 2, MPI_INT, 0, MCW);

    int rows = dimensions[0];
    int columns = dimensions[1];
    int root = process_count - 1;
    if (columns <= 0 || rows <= 0 ||
        process_count >= (int)(sizeof(unsigned int) * CHAR_BIT) ||
        (unsigned int)rows != (1U << process_count) - 1 ||
        columns > INT_MAX / (1 << root)) {
        if (rank == 0)
            fprintf(stderr, "Dimensions do not fit the row partition or MPI count.\n");
        MPI_Finalize();
        return 1;
    }

    int local_rows = 1 << rank;
    int *vector = malloc((size_t)columns * sizeof *vector);
    int *local_matrix;
    if (rank == 0) {
        size_t matrix_count = (size_t)rows * columns;
        local_matrix = malloc(matrix_count * sizeof *local_matrix);
        for (size_t i = 0; i < matrix_count; i++)
            scanf("%d", &local_matrix[i]);
        for (int j = 0; j < columns; j++)
            scanf("%d", &vector[j]);

        for (int process = 1; process < process_count; process++) {
            int block_rows = 1 << process;
            size_t offset = (size_t)(block_rows - 1) * columns;
            MPI_Send(local_matrix + offset, block_rows * columns,
                     MPI_INT, process, 0, MCW);
        }
    } else {
        int local_count = local_rows * columns;
        local_matrix = malloc((size_t)local_count * sizeof *local_matrix);
        MPI_Recv(local_matrix, local_count, MPI_INT, 0, 0, MCW, MPI_STATUS_IGNORE);
    }
    MPI_Bcast(vector, columns, MPI_INT, 0, MCW);

    int result_rows = rank == root ? rows : local_rows;
    int *result = calloc((size_t)result_rows, sizeof *result);
    int *local_result = rank == root ? result + local_rows - 1 : result;
    for (int i = 0; i < local_rows; i++)
        for (int j = 0; j < columns; j++)
            local_result[i] += local_matrix[i * columns + j] * vector[j];

    if (rank != root) {
        MPI_Send(local_result, local_rows, MPI_INT, root, 1, MCW);
    } else {
        for (int process = 0; process < root; process++) {
            int block_rows = 1 << process;
            MPI_Recv(result + block_rows - 1, block_rows, MPI_INT,
                     process, 1, MCW, MPI_STATUS_IGNORE);
        }

        printf("Result on process %d:", rank);
        for (int i = 0; i < rows; i++)
            printf(" %d", result[i]);
        printf("\n");
    }

    free(result);
    free(local_matrix);
    free(vector);
    MPI_Finalize();
    return 0;
}
