#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

enum { N = 6, MARKED_COUNT = 15 };

/*
 * Ispit 20. 9. 2024, zadatak 1b.
 * Napraviti izvedeni tip koji obuhvata označene elemente celobrojne
 * kvadratne matrice A sa slike. Master jednim pozivom funkcije šalje
 * te elemente procesima P1, P3 i P5. Primaoci štampaju potvrdu prijema.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Group world_group, recipient_group;
    MPI_Comm recipients;
    int recipient_ranks[] = {0, 1, 3, 5};
    int a[N][N];
    int lengths[MARKED_COUNT];
    /* Marked offsets from the figure, listed row by row. */
    int displacements[MARKED_COUNT] = {
        1, 2, 4,
        8, 9, 11,
        15, 16,
        18, 22, 23,
        25, 29,
        30, 32
    };

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (size < 6)
    {
        if (rank == 0)
            fprintf(stderr, "At least 6 processes are required.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, 4, recipient_ranks, &recipient_group);
    MPI_Comm_create(MCW, recipient_group, &recipients);

    if (rank == 0 || rank == 1 || rank == 3 || rank == 5)
    {
        for (int i = 0; i < MARKED_COUNT; i++)
            lengths[i] = 1;

        if (rank == 0)
            for (int row = 0; row < N; row++)
                for (int col = 0; col < N; col++)
                    a[row][col] = row * N + col;

        MPI_Datatype marked_type;
        MPI_Type_indexed(MARKED_COUNT, lengths, displacements, MPI_INT, &marked_type);
        MPI_Type_commit(&marked_type);
        MPI_Bcast(a, 1, marked_type, 0, recipients);

        if (rank != 0)
            printf("Process %d received all %d marked elements.\n", rank, MARKED_COUNT);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
