#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

enum { K = 2, M = 4, L = 2 };

/*
 * Napisati MPI program koji množi matricu A dimenzija k × m i matricu B
 * dimenzija m × l, čime se dobija rezultujuća matrica C.
 *
 * Matrice A i B inicijalizuju se u master procesu. Za p procesa, pri čemu
 * je m deljivo sa p, proces Pi dobija kolone matrice A i vrste matrice B
 * sa indeksima i, i + p, i + 2p, ..., i + m - p (0 ≤ i < p).
 * Master distribuira blokove tako da se slanje odgovarajućeg bloka svake
 * matrice svakom procesu obavlja odjednom.
 *
 * Svaki proces obavlja odgovarajuća izračunavanja i učestvuje u generisanju
 * rezultata. Rezultat se prikazuje u procesu koji nakon raspodele sadrži
 * minimum matrice A. Zadatak rešiti korišćenjem grupnih operacija.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    double a[K][M], b[M][L], c[K][L];
    double local_c[K][L] = {0};
    MPI_Datatype a_vector, a_block, b_vector, b_block;
    struct {
        double value;
        int rank;
    } local_min, global_min;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (M % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "M (%d) must be divisible by the number of processes (%d).\n", M, size);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_width = M / size;
    double local_a[K][local_width];
    double local_b[local_width][L];

    if (rank == 0)
    {
        const double initial_a[K][M] = {{2, 1, 4, 3}, {3, 4, 5, 6}};
        const double initial_b[M][L] = {{1, 0}, {0, 1}, {1, 0}, {0, 1}};
        memcpy(a, initial_a, sizeof a);
        memcpy(b, initial_b, sizeof b);
    }

    MPI_Type_vector(K * local_width, 1, size, MPI_DOUBLE, &a_vector);
    MPI_Type_create_resized(a_vector, 0, sizeof(double), &a_block);
    MPI_Type_commit(&a_block);

    MPI_Type_vector(local_width, L, size * L, MPI_DOUBLE, &b_vector);
    MPI_Type_create_resized(b_vector, 0, L * sizeof(double), &b_block);
    MPI_Type_commit(&b_block);

    MPI_Scatter(a, 1, a_block, local_a, K * local_width, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, 1, b_block, local_b, local_width * L, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    local_min.value = local_a[0][0];
    local_min.rank = rank;
    for (int row = 0; row < K; row++)
        for (int col = 0; col < local_width; col++)
            if (local_a[row][col] < local_min.value)
                local_min.value = local_a[row][col];

    for (int row = 0; row < K; row++)
        for (int col = 0; col < L; col++)
            for (int inner = 0; inner < local_width; inner++)
                local_c[row][col] += local_a[row][inner] * local_b[inner][col];

    MPI_Reduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
    MPI_Bcast(&global_min.rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Reduce(local_c, c, K * L, MPI_DOUBLE, MPI_SUM, global_min.rank, MPI_COMM_WORLD);

    if (rank == global_min.rank)
    {
        printf("Process %d holds the minimum of A (%g).\n", rank, local_min.value);
        print_matrix("Matrix C", K, L, &c[0][0]);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
