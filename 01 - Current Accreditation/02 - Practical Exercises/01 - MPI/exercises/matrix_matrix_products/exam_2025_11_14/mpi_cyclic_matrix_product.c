#include <mpi.h>
#include <stdio.h>

enum { N = 2, K = 4, M = 2 };

/*
 * 14.11.2025. — 6a
 * Napisati MPI program koji množi matrice A (n × k) i B (k × m)
 * inicijalizovane u master procesu. Broj k je deljiv brojem procesa p.
 * Proces Pi dobija kolone A i vrste B sa indeksima l za koje važi l mod p = i.
 * Sve kolone A i sve vrste B namenjene jednom procesu šalju se odjednom,
 * direktno iz odgovarajuće matrice. Svi procesi računaju svoje doprinose
 * i učestvuju u formiranju rezultata. Koristiti grupne operacije.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (K % size != 0) {
        if (rank == 0)
            fprintf(stderr, "The inner dimension must be divisible by the process count.\n");
        MPI_Finalize();
        return 1;
    }

    int a[N][K], b[K][M], c[N][M];
    if (rank == 0) {
        for (int row = 0; row < N; row++)
            for (int col = 0; col < K; col++)
                a[row][col] = row * K + col + 1;

        for (int row = 0; row < K; row++)
            for (int col = 0; col < M; col++)
                b[row][col] = row * M + col + 1;
    }

    int width = K / size;
    int local_a[N][width], local_b[width][M];
    int local_c[N][M] = {0};
    MPI_Datatype a_vector, a_block, b_vector, b_block;

    MPI_Type_vector(N * width, 1, size, MPI_INT, &a_vector);
    MPI_Type_create_resized(a_vector, 0, sizeof(int), &a_block);
    MPI_Type_commit(&a_block);

    MPI_Type_vector(width, M, size * M, MPI_INT, &b_vector);
    MPI_Type_create_resized(b_vector, 0, M * sizeof(int), &b_block);
    MPI_Type_commit(&b_block);

    MPI_Scatter(a, 1, a_block, local_a, N * width, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, 1, b_block, local_b, width * M, MPI_INT, 0, MPI_COMM_WORLD);

    for (int row = 0; row < N; row++)
        for (int col = 0; col < M; col++)
            for (int inner = 0; inner < width; inner++)
                local_c[row][col] += local_a[row][inner] * local_b[inner][col];

    MPI_Reduce(local_c, c, N * M, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        puts("Matrix C:");
        for (int row = 0; row < N; row++)
            for (int col = 0; col < M; col++)
                printf("%d%c", c[row][col], col == M - 1 ? '\n' : ' ');
    }

    MPI_Finalize();
    return 0;
}
