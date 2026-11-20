#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

#ifndef N
#define N 6
#endif

_Static_assert(N > 0 && N % 2 == 0, "N must be positive and even.");

/* Ispit 19.06.2017, zadatak 4a.
 * Napisati MPI program kojim se elementi matrice C procesa sa rangom
 * i (i != 0) formiraju slanjem i primanjem podataka direktno iz
 * matrice A dimenzija n x n (n je parno) procesa sa rangom 0,
 * na način prikazan za n = 6:
 *
 * 0 0 0   0   0   a41
 * 0 0 0   0   a42 a40
 * 0 0 0   a24 a23 a21
 * 0 0 a45 a05 a22 a20
 * 0 0 a44 a04 a03 a01
 * 0 0 a25 a43 a02 a00
 *
 * Ostali elementi matrice C jednaki su nuli. P0 sa svakim procesom
 * komunicira samo jednim slanjem/primanjem podataka.
 */
int main(int argc, char **argv)
{
    enum { pair_count = (N / 2) * (N / 2), element_count = 2 * pair_count };
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0) {
        int a[N][N];
        int lengths[pair_count], displacements[pair_count];
        int index = 0;
        MPI_Datatype send_type;

        for (int row = 0; row < N; row++)
            for (int col = 0; col < N; col++)
                a[row][col] = row * N + col + 1;

        for (int col = 0; col < N; col += 2) {
            for (int row = 0; row < N; row += 2) {
                lengths[index] = 2;
                displacements[index] = row * N + col;
                index++;
            }
        }

        MPI_Type_indexed(pair_count, lengths, displacements, MPI_INT,
                         &send_type);
        MPI_Type_commit(&send_type);

        for (int destination = 1; destination < size; destination++)
            MPI_Send(a, 1, send_type, destination, 0, MCW);
    } else {
        int c[N][N] = {0};
        int lengths[element_count], displacements[element_count];
        int index = 0;
        MPI_Datatype receive_type;

        for (int col = N - 1; col >= 0 && index < element_count; col--) {
            for (int row = N - 1; row >= N - 1 - col && index < element_count;
                 row--) {
                lengths[index] = 1;
                displacements[index] = row * N + col;
                index++;
            }
        }

        MPI_Type_indexed(element_count, lengths, displacements, MPI_INT,
                         &receive_type);
        MPI_Type_commit(&receive_type);
        MPI_Recv(c, 1, receive_type, 0, 0, MCW, MPI_STATUS_IGNORE);

        printf("C on process %d:\n", rank);
        for (int row = 0; row < N; row++)
            for (int col = 0; col < N; col++)
                printf("%d%c", c[row][col], col == N - 1 ? '\n' : ' ');
    }

    MPI_Finalize();
    return 0;
}
