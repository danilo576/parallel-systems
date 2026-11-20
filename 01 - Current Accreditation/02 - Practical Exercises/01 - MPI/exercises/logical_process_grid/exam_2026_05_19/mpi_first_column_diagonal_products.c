#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define N 4

/*
 * 19.05.2026, zadatak 6b.
 * Napisati MPI program koji kreira komunikator samo od procesa u prvoj
 * koloni kvadratne mreže dimenzija sqrt(p) x sqrt(p), gde je p broj procesa.
 * Master P0 grupnom operacijom svakom procesu tog komunikatora šalje
 * jednak broj elemenata glavne dijagonale matrice X dimenzija n x n.
 * Broj n je deljiv sa sqrt(p). U svakom procesu komunikatora prikazati
 * identifikator procesa i proizvod primljenih elemenata.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Group world_group, column_group;
    MPI_Comm column_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int side = (int)sqrt(size);
    if (side * side != size || N % side != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The process count must be a perfect square and its square root must divide N.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int members[side];
    for (int i = 0; i < side; i++)
        members[i] = i * side;

    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, side, members, &column_group);
    MPI_Comm_create(MCW, column_group, &column_comm);

    if (rank % side == 0)
    {
        int column_rank;
        int x[N][N] = {{0}};
        int count = N / side;
        int local_diagonal[count];
        MPI_Datatype diagonal_type, scatter_type;

        MPI_Comm_rank(column_comm, &column_rank);

        if (column_rank == 0)
            for (int i = 0; i < N; i++)
                x[i][i] = i + 1;

        MPI_Type_vector(count, 1, N + 1, MPI_INT, &diagonal_type);
        MPI_Type_create_resized(diagonal_type, 0,
                                (MPI_Aint)count * (N + 1) * sizeof(int),
                                &scatter_type);
        MPI_Type_commit(&scatter_type);
        MPI_Scatter(x, 1, scatter_type, local_diagonal, count, MPI_INT, 0, column_comm);

        long long product = 1;
        for (int i = 0; i < count; i++)
            product *= local_diagonal[i];

        printf("Process %d (column rank %d): product = %lld\n",
               rank, column_rank, product);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
