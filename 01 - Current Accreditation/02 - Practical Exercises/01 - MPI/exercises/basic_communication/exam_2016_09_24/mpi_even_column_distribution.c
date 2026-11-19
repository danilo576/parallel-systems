#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

#ifndef N
#define N 6
#endif

#if N < 2 || N % 2 != 0
#error N must be positive and even
#endif

/* Ispit 24.09.2016, zadatak 4(a).
 * Napisati MPI program kojim svaki proces sa rangom i != 0 formira
 * matricu C od elemenata matrice A reda n procesa P0, gde je n parno.
 * Za n = 6 raspored je:
 *
 * 0 0 a50 a34 a20 a00
 * 0 0 a52 a40 a22 a02
 * 0 0 a54 a42 a24 a04
 * 0 0 0   a44 a30 a10
 * 0 0 0   0   a32 a12
 * 0 0 0   0   0   a14
 *
 * Ostali elementi matrice C jednaki su nuli. P0 sa svakim procesom
 * komunicira samo jednim slanjem/primanjem podataka.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    const int count = N * (N / 2);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0) {
        int a[N * N];
        MPI_Datatype send_type;

        for (int index = 0; index < N * N; index++)
            a[index] = index + 1;

        MPI_Type_vector(count, 1, 2, MPI_INT, &send_type);
        MPI_Type_commit(&send_type);
        for (int destination = 1; destination < size; destination++)
            MPI_Send(a, 1, send_type, destination, 0, MCW);
    } else {
        int c[N * N] = {0};
        int lengths[count], displacements[count];
        int index = 0;
        MPI_Datatype receive_type;

        for (int col = N - 1; index < count; col--) {
            for (int row = 0; row <= col && index < count; row++) {
                lengths[index] = 1;
                displacements[index] = row * N + col;
                index++;
            }
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
