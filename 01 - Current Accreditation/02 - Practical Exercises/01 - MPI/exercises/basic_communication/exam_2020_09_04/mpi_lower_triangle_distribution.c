#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

#ifndef N
#define N 4
#endif

#if N < 1
#error N must be positive
#endif

/* Ispit 04.09.2020, zadatak 4(a).
 * Napisati MPI program kojim se elementi matrice C na svakom procesu
 * sa rangom i != 0 formiraju od elemenata matrice A procesa P0,
 * prema primeru za n = 4:
 *
 * a00 a21 a32 0
 * a10 a22 a33 0
 * a11 a30 0   0
 * a20 a31 0   0
 *
 * Ostali elementi matrice C jednaki su nuli. P0 sa svakim procesom
 * komunicira samo jednim slanjem/primanjem podataka.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0) {
        int a[N * N];
        int lengths[N], displacements[N];
        MPI_Datatype send_type;

        for (int index = 0; index < N * N; index++)
            a[index] = index + 1;

        for (int row = 0; row < N; row++) {
            lengths[row] = row + 1;
            displacements[row] = row * N;
        }

        MPI_Type_indexed(N, lengths, displacements, MPI_INT, &send_type);
        MPI_Type_commit(&send_type);

        for (int destination = 1; destination < size; destination++)
            MPI_Send(a, 1, send_type, destination, 0, MCW);
    } else {
        const int count = N * (N + 1) / 2;
        int c[N * N] = {0};
        int lengths[count], displacements[count];
        MPI_Datatype receive_type;

        for (int index = 0; index < count; index++) {
            lengths[index] = 1;
            displacements[index] = (index % N) * N + index / N;
        }

        MPI_Type_indexed(count, lengths, displacements, MPI_INT, &receive_type);
        MPI_Type_commit(&receive_type);
        MPI_Recv(c, 1, receive_type, 0, 0, MCW, MPI_STATUS_IGNORE);

        printf("C on process %d:\n", rank);
        for (int row = 0; row < N; row++) {
            for (int col = 0; col < N; col++)
                printf("%d%c", c[row * N + col], col == N - 1 ? '\n' : ' ');
        }
    }

    MPI_Finalize();
    return 0;
}
