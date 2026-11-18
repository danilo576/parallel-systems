#include <math.h>
#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

/*
 * 14.11.2025. — 6b
 * Napisati MPI program koji kreiranjem komunikatora omogućava komunikaciju
 * samo procesima u donjem trouglu kvadratne mreže procesa. Obezbediti da svi
 * procesi te grupe dobiju sve elemente glavne dijagonale matrice A direktno
 * iz matrice.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int q = (int)sqrt(size);

    int row = rank / q;
    int col = rank % q;
    int lower_ranks[size];
    int lower_count = 0;
    for (int i = 0; i < q; i++)
        for (int j = 0; j < q; j++)
            if (i >= j)
                lower_ranks[lower_count++] = i * q + j;

    MPI_Group world_group, lower_group;
    MPI_Comm lower_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, lower_count, lower_ranks, &lower_group);
    MPI_Comm_create(MCW, lower_group, &lower_comm);

    if (row >= col) {
        int a[q][q];
        if (rank == 0) {
            for (int i = 0; i < q; i++)
                for (int j = 0; j < q; j++)
                    a[i][j] = i * q + j + 1;
        }

        MPI_Datatype diagonal_type;
        MPI_Type_vector(q, 1, q + 1, MPI_INT, &diagonal_type);
        MPI_Type_commit(&diagonal_type);
        MPI_Bcast(a, 1, diagonal_type, 0, lower_comm);

        printf("P%d diagonal:", rank);
        for (int i = 0; i < q; i++)
            printf(" %d", a[i][i]);
        putchar('\n');
    }

    MPI_Finalize();
    return 0;
}
