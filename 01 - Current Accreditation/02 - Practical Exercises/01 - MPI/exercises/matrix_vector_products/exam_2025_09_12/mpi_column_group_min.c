#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define M 4
#define K 4

/*
 * 12.09.2025, zadatak 1b
 *
 * Napisati MPI program koji kreira komunikator svih procesa sa
 * identifikatorima deljivim zadatom vrednošću x. Svaki proces grupe
 * dobija po jednu kolonu matrice A. Odštampati identifikatore članova
 * novog komunikatora i minimum suma primljenih kolona matrice A.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    const int x = 2;
    MPI_Group world_group, column_group;
    MPI_Comm column_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int members[size];
    int member_count = 0;
    for (int member = 0; member < size; member++)
        if (member % x == 0)
            members[member_count++] = member;

    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, member_count, members, &column_group);
    MPI_Comm_create(MCW, column_group, &column_comm);

    if (rank % x == 0) {
        int a[M][K], local_column[M], minimum;
        MPI_Datatype column_stride, column_type;

        if (rank == 0)
            for (int row = 0; row < M; row++)
                for (int col = 0; col < K; col++)
                    a[row][col] = row * K + col + 1;

        MPI_Type_vector(M, 1, K, MPI_INT, &column_stride);
        MPI_Type_create_resized(column_stride, 0, sizeof(int), &column_type);
        MPI_Type_commit(&column_type);
        MPI_Scatter(a, 1, column_type, local_column, M, MPI_INT, 0, column_comm);

        int sum = 0;
        for (int row = 0; row < M; row++)
            sum += local_column[row];

        MPI_Reduce(&sum, &minimum, 1, MPI_INT, MPI_MIN, 0, column_comm);

        if (rank == 0) {
            printf("Group ranks in MCW:");
            for (int i = 0; i < member_count; i++)
                printf(" %d", members[i]);
            printf("\nMinimum column sum: %d\n", minimum);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
