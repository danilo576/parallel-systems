#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

#ifndef N
#define N 6
#endif

#if N < 2 || N % 2 != 0
#error N must be positive and even
#endif

/* Ispit 08.09.2016, zadatak 4(a).
 * Napisati MPI program kojim se elementi matrice C svakog procesa
 * sa rangom i (i != 0) formiraju slanjem i primanjem podataka iz matrice
 * A reda n procesa sa rangom 0, pri čemu je n parno.
 * Za n = 6 raspored u matrici C je:
 *
 * a00 a02 a04 0 0 0
 * a01 a03 a05 0 0 0
 * a20 a22 a24 0 0 0
 * a21 a23 a25 0 0 0
 * a40 a42 a44 0 0 0
 * a41 a43 a45 0 0 0
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0) {
        int a[N * N];
        MPI_Datatype send_type;

        for (int index = 0; index < N * N; index++)
            a[index] = index + 1;

        MPI_Type_vector(N / 2, N, 2 * N, MPI_INT, &send_type);
        MPI_Type_commit(&send_type);

        for (int destination = 1; destination < size; destination++)
            MPI_Send(a, 1, send_type, destination, 0, MCW);
    } else {
        int c[N * N] = {0};
        int lengths[N * N / 2], displacements[N * N / 2];
        int index = 0;
        MPI_Datatype receive_type;

        for (int row = 0; row < N; row += 2) {
            for (int col = 0; col < N / 2; col++) {
                lengths[index] = 1;
                displacements[index++] = row * N + col;
                lengths[index] = 1;
                displacements[index++] = (row + 1) * N + col;
            }
        }

        MPI_Type_indexed(N * N / 2, lengths, displacements, MPI_INT, &receive_type);
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
