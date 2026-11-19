#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

enum { N = 2, S = 2, K = 4 };

/*
 * 26.02.2026. — 6a
 * Napisati MPI program koji množi matrice A (n × s) i B (s × k)
 * i računa proizvod elemenata svake vrste matrice B. Master
 * inicijalizuje matrice i svim procesima šalje celu matricu A.
 * Broj k je deljiv brojem procesa p. Proces Pi odjednom dobija
 * kolone i, i+p, ..., i+k-p direktno iz matrice B. Svaki proces
 * računa svoj deo i učestvuje u formiranju rezultata. Prikazati
 * rezultate u proizvoljnom procesu. Koristiti grupne operacije.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K % size != 0) {
        if (rank == 0)
            fprintf(stderr, "The column count must be divisible by the process count.\n");
        MPI_Finalize();
        return 1;
    }

    int result_rank = size - 1;
    int a[N][S], b[S][K], c[N][K];
    if (rank == 0) {
        for (int row = 0; row < N; row++)
            for (int col = 0; col < S; col++)
                a[row][col] = row * S + col + 1;

        for (int row = 0; row < S; row++)
            for (int col = 0; col < K; col++)
                b[row][col] = row + col + 1;
    }

    int width = K / size;
    int local_b[S][width], local_c[N][width];
    long long local_products[S], row_products[S];
    MPI_Datatype b_vector, b_block, c_vector, c_block;

    MPI_Type_vector(S * width, 1, size, MPI_INT, &b_vector);
    MPI_Type_create_resized(b_vector, 0, sizeof(int), &b_block);
    MPI_Type_commit(&b_block);

    MPI_Type_vector(N * width, 1, size, MPI_INT, &c_vector);
    MPI_Type_create_resized(c_vector, 0, sizeof(int), &c_block);
    MPI_Type_commit(&c_block);

    MPI_Bcast(a, N * S, MPI_INT, 0, MCW);
    MPI_Scatter(b, 1, b_block, local_b, S * width, MPI_INT, 0, MCW);

    for (int row = 0; row < N; row++)
        for (int col = 0; col < width; col++) {
            local_c[row][col] = 0;
            for (int inner = 0; inner < S; inner++)
                local_c[row][col] += a[row][inner] * local_b[inner][col];
        }

    for (int row = 0; row < S; row++) {
        local_products[row] = 1;
        for (int col = 0; col < width; col++)
            local_products[row] *= local_b[row][col];
    }

    MPI_Gather(local_c, N * width, MPI_INT, c, 1, c_block, result_rank, MCW);
    MPI_Reduce(local_products, row_products, S, MPI_LONG_LONG_INT, MPI_PROD, result_rank, MCW);

    if (rank == result_rank) {
        printf("Process %d\nMatrix C:\n", rank);
        for (int row = 0; row < N; row++)
            for (int col = 0; col < K; col++)
                printf("%d%c", c[row][col], col == K - 1 ? '\n' : ' ');

        puts("Row products of B:");
        for (int row = 0; row < S; row++)
            printf("%lld%c", row_products[row], row == S - 1 ? '\n' : ' ');
    }

    MPI_Finalize();
    return 0;
}
