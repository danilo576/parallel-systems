#include <mpi.h>
#include <stdio.h>

#ifndef N
#define N 4
#endif

#if N < 2 || N % 2 != 0
#error N must be positive and even
#endif

static void print_matrix(const char *name, int rank, const int matrix[])
{
    printf("%s on process %d:\n", name, rank);
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++)
            printf("%d%c", matrix[row * N + col], col == N - 1 ? '\n' : ' ');
    }
}

/*
 * Ispit 08.09.2017, zadatak 4(a).
 * Napisati MPI program kojim procesi P1, P2 i P3 formiraju svoje matrice
 * C reda n od elemenata matrice A reda n na procesu P0, gde je n parno.
 * P1 dobija gornji desni kvadrant (i < n/2, j >= n/2), P2 donji levi
 * (i >= n/2, j < n/2), a P3 donji desni (i >= n/2, j >= n/2).
 * Prijem je isti za sve procese. Za n = 4, P1 formira sledeću matricu C:
 *
 * 0   0   0   a02
 * 0   0   a03 a12
 * 0   a13 a02 a03
 * a12 a13 0   0
 *
 * Ostali elementi matrice C jednaki su nuli. P0 sa svakim procesom
 * komunicira samo jednim slanjem/primanjem podataka.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    const int half = N / 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0)
            fprintf(stderr, "Run this program with exactly 4 processes.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        int a[N * N];
        MPI_Datatype quadrant, repeated_quadrant;

        for (int index = 0; index < N * N; index++)
            a[index] = index + 1;

        MPI_Type_vector(half, half, N, MPI_INT, &quadrant);
        /* A zero extent makes both instances start at the same address. */
        MPI_Type_create_resized(quadrant, 0, 0, &repeated_quadrant);
        MPI_Type_commit(&repeated_quadrant);

        print_matrix("A", rank, a);
        for (int destination = 1; destination < size; destination++) {
            int first_row = (destination / 2) * half;
            int first_col = (destination % 2) * half;
            MPI_Send(a + first_row * N + first_col, 2, repeated_quadrant,
                     destination, 0, MPI_COMM_WORLD);
        }
    } else {
        int c[N * N] = {0};
        int lengths[N], displacements[N];
        int remaining = 2 * half * half;
        int blocks = 0;
        MPI_Datatype receive_type;

        for (int row = 0; remaining > 0; row++) {
            int first_col = N - 1 - row;
            int length = row + 1 < remaining ? row + 1 : remaining;
            lengths[blocks] = length;
            displacements[blocks] = row * N + first_col;
            blocks++;
            remaining -= length;
        }

        MPI_Type_indexed(blocks, lengths, displacements, MPI_INT,
                         &receive_type);
        MPI_Type_commit(&receive_type);
        MPI_Recv(c, 1, receive_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        print_matrix("C", rank, c);
    }

    MPI_Finalize();
    return 0;
}
