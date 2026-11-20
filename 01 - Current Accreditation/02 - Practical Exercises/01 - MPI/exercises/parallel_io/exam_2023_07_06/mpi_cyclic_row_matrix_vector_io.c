#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define K 6
#define N 3

/*
 * Ispit 06.07.2023, zadatak 1.
 * Napisati MPI program za množenje matrice A dimenzija k x n i vektora b
 * dužine n. Master inicijalizuje A i b. Ako je p broj procesa i k je
 * deljivo sa p, proces Pi dobija vrste i, i+p, ..., i+k-p matrice A.
 * Master šalje sve vrste za isti proces odjednom, direktno iz matrice,
 * i distribuira ceo vektor b svim procesima.
 * Svi procesi, uključujući master, računaju svoje elemente rezultata
 * i paralelno ih upisuju u binarnu datoteku rez.dat u globalnom poretku.
 * Koristiti grupne operacije, izvedene tipove i funkcije za rad sa datotekama.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int *a = NULL;
    int b[N];
    MPI_Datatype row_type, scatter_type, file_type;
    MPI_File file;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The number of rows must be divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int rows = K / size;
    int local_a[rows][N];
    int local_c[rows];

    if (rank == 0)
    {
        a = malloc((size_t)K * N * sizeof(*a));
        for (int i = 0; i < K; i++)
            for (int j = 0; j < N; j++)
                a[i * N + j] = i * N + j + 1;
        for (int j = 0; j < N; j++)
            b[j] = j + 1;
    }

    MPI_Type_vector(rows, N, size * N, MPI_INT, &row_type);
    MPI_Type_create_resized(row_type, 0, N * sizeof(int), &scatter_type);
    MPI_Type_commit(&scatter_type);

    MPI_Scatter(a, 1, scatter_type, local_a, rows * N, MPI_INT, 0, MCW);
    MPI_Bcast(b, N, MPI_INT, 0, MCW);
    free(a);

    for (int i = 0; i < rows; i++)
    {
        local_c[i] = 0;
        for (int j = 0; j < N; j++)
            local_c[i] += local_a[i][j] * b[j];
    }

    MPI_Type_vector(rows, 1, size, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);
    MPI_File_open(MCW, "rez.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, (MPI_Offset)K * sizeof(int));
    MPI_File_set_view(file, (MPI_Offset)rank * sizeof(int), MPI_INT,
                      file_type, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, local_c, rows, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
