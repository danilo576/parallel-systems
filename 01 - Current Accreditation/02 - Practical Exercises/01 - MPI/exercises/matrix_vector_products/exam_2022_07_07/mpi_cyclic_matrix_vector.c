#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define M 2
#define N 4
#define ROOT 0

/*
 * 07.07.2022, zadatak 2a
 *
 * Napisati MPI program koji množi matricu A dimenzija m × n vektorom b
 * dužine n i dobija vektor c. Master proces inicijalizuje A i b.
 * Za p procesa, gde je n deljivo sa p, proces P_i dobija kolone matrice A
 * i elemente vektora b sa indeksima i, i+p, i+2p, ..., i+n-p.
 * Master distribuira svaki blok matrice i svaki deo vektora odjednom
 * u odgovarajućem slanju. Svaki proces obavlja lokalna izračunavanja
 * i učestvuje u formiranju rezultata. Koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int a[M][N], b[N], c[M];
    int local_c[M] = {0};
    MPI_Datatype matrix_stride, vector_stride, matrix_type, vector_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0) {
        if (rank == ROOT)
            fprintf(stderr, "N must be divisible by the number of processes.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int local_columns = N / size;
    int local_a[M][local_columns];
    int local_b[local_columns];

    if (rank == ROOT) {
        for (int row = 0; row < M; row++)
            for (int col = 0; col < N; col++)
                a[row][col] = row * N + col + 1;

        for (int col = 0; col < N; col++)
            b[col] = col + 1;
    }

    MPI_Type_vector(M * local_columns, 1, size, MPI_INT, &matrix_stride);
    MPI_Type_create_resized(matrix_stride, 0, sizeof(int), &matrix_type);
    MPI_Type_commit(&matrix_type);

    MPI_Type_vector(local_columns, 1, size, MPI_INT, &vector_stride);
    MPI_Type_create_resized(vector_stride, 0, sizeof(int), &vector_type);
    MPI_Type_commit(&vector_type);

    MPI_Scatter(a, 1, matrix_type, local_a, M * local_columns, MPI_INT,
                ROOT, MPI_COMM_WORLD);
    MPI_Scatter(b, 1, vector_type, local_b, local_columns, MPI_INT,
                ROOT, MPI_COMM_WORLD);

    for (int row = 0; row < M; row++)
        for (int col = 0; col < local_columns; col++)
            local_c[row] += local_a[row][col] * local_b[col];

    MPI_Reduce(local_c, c, M, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

    if (rank == ROOT)
        for (int row = 0; row < M; row++)
            printf("c[%d] = %d\n", row, c[row]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
