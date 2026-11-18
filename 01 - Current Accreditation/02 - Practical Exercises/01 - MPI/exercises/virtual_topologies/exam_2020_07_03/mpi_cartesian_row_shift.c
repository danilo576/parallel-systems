#include <mpi.h>
#include <stdio.h>

enum { ROWS = 3, COLS = 4 };
/*
 * 3. 7. 2020. — Zadatak 4(a)
 *
 * Napisati deo MPI koda koji formira komunikator Kartezijanske topologije
 * tako da sadržaj promenljive x u svakom procesu pre i nakon poziva
 * MPI_Sendrecv_replace(&x, 1, MPI_INT, dest, 0, source, 0, cartcomm, &st)
 * izgleda kao u prikazanom primeru komunikatora 3 × 4.
 *
 * Pre:       Posle:
 * 0 1 2 3    0 1 2 3
 * 1 2 3 4    4 1 2 3
 * 2 3 4 5    4 5 2 3
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != ROWS * COLS) {
        if (rank == 0)
            fprintf(stderr, "Run with exactly %d processes.\n", ROWS * COLS);
        MPI_Finalize();
        return 0;
    }

    int dims[2] = {ROWS, COLS};
    int periods[2] = {0, 1};
    MPI_Comm cartcomm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cartcomm);

    int coords[2], source, dest;
    MPI_Cart_coords(cartcomm, rank, 2, coords);
    int row = coords[0];
    int col = coords[1];
    int x = row + col;
    int before = x;

    MPI_Cart_shift(cartcomm, 1, row, &source, &dest);

    MPI_Status st;
    MPI_Sendrecv_replace(&x, 1, MPI_INT, dest, 0, source, 0, cartcomm, &st);

    printf("P%d (%d, %d): %d -> %d\n", rank, row, col, before, x);

    MPI_Finalize();
    return 0;
}
