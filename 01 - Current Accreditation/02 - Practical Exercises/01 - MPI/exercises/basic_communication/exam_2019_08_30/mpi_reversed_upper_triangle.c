#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

#ifndef N
#define N 4
#endif

/* Ispit 30.08.2019, zadatak 4a.
 * Napisati MPI program kojim se elementi matrice C svakog procesa
 * sa rangom i (i != 0) formiraju slanjem i primanjem elemenata
 * matrice A procesa P0, prema sledećem rasporedu za n = 4:
 *
 * a20 a11 a10 a00
 * 0   a30 a22 a21
 * 0   0   a32 a31
 * 0   0   0   a33
 *
 * Ostali elementi C jednaki su nuli. P0 sa svakim procesom
 * komunicira samo jednim slanjem/primanjem podataka.
 */
int main(int argc, char **argv)
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0) {
        int a[N][N];
        int lengths[N], displacements[N];
        MPI_Datatype send_type;

        for (int row = 0; row < N; row++) {
            for (int col = 0; col < N; col++)
                a[row][col] = row * N + col + 1;

            lengths[row] = row + 1;
            displacements[row] = row * N;
        }

        MPI_Type_indexed(N, lengths, displacements, MPI_INT, &send_type);
        MPI_Type_commit(&send_type);

        for (int destination = 1; destination < size; destination++)
            MPI_Send(a, 1, send_type, destination, 0, MCW);
    } else {
        enum { element_count = N * (N + 1) / 2 };
        int c[N][N] = {0};
        int lengths[element_count], displacements[element_count];
        int index = 0;
        MPI_Datatype receive_type;

        for (int row = 0; row < N; row++) {
            for (int col = N - 1; col >= row; col--) {
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
        for (int row = 0; row < N; row++) {
            for (int col = 0; col < N; col++)
                printf("%d%c", c[row][col], col == N - 1 ? '\n' : ' ');
        }
    }

    MPI_Finalize();
    return 0;
}
