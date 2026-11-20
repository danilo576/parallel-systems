#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define K 2
#define M 6

/*
 * Ispit 04.12.2023, zadatak 1.
 * Napisati MPI program za množenje matrice A dimenzija k x m i vektora
 * B dužine m. Master inicijalizuje A. Ako p deli m, proces Pi odjednom
 * dobija kolone i, i+p, ..., i+m-p i jednim efikasnim čitanjem iz fajla
 * B-total.dat učitava elemente B sa istim indeksima.
 * Svi procesi računaju svoje doprinose rezultatu. Konačan vektor treba
 * da se nađe u procesu koji je učitao maksimalni element vektora B.
 * Koristiti grupne operacije, izvedene tipove i funkcije za rad sa
 * fajlovima. Dati primer A i B, ilustrovati raspodelu i navesti komandu
 * za pokretanje programa za izabrane dimenzije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int *a = NULL;
    int local_result[K] = {0}, result[K];
    MPI_Datatype scatter_type, file_type;
    MPI_File file;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (M % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The number of columns must be divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_cols = M / size;
    int local_a[K][local_cols], local_b[local_cols];

    if (rank == 0)
    {
        a = malloc((size_t)K * M * sizeof(*a));
        for (int i = 0; i < K; i++)
            for (int j = 0; j < M; j++)
                a[i * M + j] = i * M + j + 1;
    }

    MPI_Type_vector(local_cols, 1, size, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);

    /* The stride also crosses row boundaries because M is divisible by size. */
    MPI_Datatype matrix_type;
    MPI_Type_vector(K * local_cols, 1, size, MPI_INT, &matrix_type);
    MPI_Type_create_resized(matrix_type, 0, sizeof(int), &scatter_type);
    MPI_Type_commit(&scatter_type);

    MPI_Scatter(a, 1, scatter_type, local_a, K * local_cols, MPI_INT, 0, MCW);
    free(a);

    MPI_File_open(MCW, "B-total.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, (MPI_Offset)rank * sizeof(int), MPI_INT,
                      file_type, "native", MPI_INFO_NULL);
    MPI_File_read_all(file, local_b, local_cols, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    for (int i = 0; i < K; i++)
        for (int j = 0; j < local_cols; j++)
            local_result[i] += local_a[i][j] * local_b[j];

    int local_max[2] = {local_b[0], rank};
    int global_max[2];
    for (int j = 1; j < local_cols; j++)
        if (local_b[j] > local_max[0])
            local_max[0] = local_b[j];

    MPI_Reduce(local_max, global_max, 1, MPI_2INT, MPI_MAXLOC, 0, MCW);
    MPI_Bcast(&global_max[1], 1, MPI_INT, 0, MCW);

    int result_rank = global_max[1];
    MPI_Reduce(local_result, result, K, MPI_INT, MPI_SUM, result_rank, MCW);

    if (rank == result_rank)
    {
        printf("Process %d: C =", rank);
        for (int i = 0; i < K; i++)
            printf(" %d", result[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
