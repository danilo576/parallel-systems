#include <math.h>
#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD
#define X 5

enum { N = 2, K = 4 };

/*
 * 25.09.2025. — 1b
 * Napisati MPI program koji omogućava komunikaciju samo procesima na
 * sporednoj dijagonali kvadratne mreže čiji je rang manji od zadate
 * vrednosti x. Svaki proces novog komunikatora dobija po jednu vrstu
 * matrice A. Prikazati identifikatore članova komunikatora, maksimum
 * proizvoda elemenata primljenih vrsta i rang procesa koji ima taj
 * maksimum u novom komunikatoru.
 */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int side = (int)sqrt(size);
    if (side * side != size) {
        if (rank == 0)
            fprintf(stderr, "The process count must be a perfect square.\n");
        MPI_Finalize();
        return 1;
    }

    int members[side], member_count = 0;
    for (int row = 0; row < side; row++) {
        int diagonal_rank = row * side + (side - 1 - row);
        if (diagonal_rank < X)
            members[member_count++] = diagonal_rank;
    }

    if (member_count == 0 && rank == 0)
        puts("No processes satisfy the condition.");

    if (member_count > N) {
        if (rank == 0)
            fprintf(stderr, "The matrix must have at least one row per member.\n");
        MPI_Finalize();
        return 1;
    }

    MPI_Group world_group, diag_group;
    MPI_Comm diag_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, member_count, members, &diag_group);
    MPI_Comm_create(MCW, diag_group, &diag_comm);

    if (rank / side + rank % side == side - 1 && rank < X) {
        int local_rank;
        MPI_Comm_rank(diag_comm, &local_rank);

        int a[N][K], row[K];
        if (local_rank == 0)
            for (int i = 0; i < N; i++)
                for (int j = 0; j < K; j++)
                    a[i][j] = i * K + j + 1;

        MPI_Scatter(a, K, MPI_INT, row, K, MPI_INT, 0, diag_comm);

        int local[2] = {1, local_rank};
        int maximum[2];
        for (int j = 0; j < K; j++)
            local[0] *= row[j];

        MPI_Reduce(local, maximum, 1, MPI_2INT, MPI_MAXLOC, 0, diag_comm);

        if (local_rank == 0) {
            printf("Member ranks (global):");
            for (int i = 0; i < member_count; i++)
                printf(" %d", members[i]);
            putchar('\n');
            printf("Maximum row product: %d\n", maximum[0]);
            printf("Process with maximum (local rank): %d\n", maximum[1]);
        }
    }

    MPI_Finalize();
    return 0;
}
