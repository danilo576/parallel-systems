#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

enum { ROWS = 4, COLS = 4 };
/*
 * 24.04.2021. — Zadatak 4(a)
 *
 * Napisati deo MPI koda koji formira komunikator Kartezijanske topologije
 * tako da sadržaj promenljive x u svakom procesu pre i nakon poziva
 * MPI_Sendrecv_replace(&x, 1, MPI_INT, dest, 0, source, 0, cartcomm, &st)
 * izgleda kao u prikazanom primeru komunikatora 4 × 4.
 *
 * Pre:       Posle:
 * 0 1 2 3    0 1 2 3
 * 1 2 3 4    2 3 4 1
 * 2 3 4 5    4 5 2 3
 * 3 4 5 6    6 3 4 5
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (size != ROWS * COLS) {
        if (rank == 0)
            fprintf(stderr, "Run with exactly %d processes.\n", ROWS * COLS);
        MPI_Finalize();
        return 0;
    }

    int dims[2] = {ROWS, COLS};
    int periods[2] = {0, 1};
    MPI_Comm cartcomm;
    MPI_Cart_create(MCW, 2, dims, periods, 0, &cartcomm);

    int coords[2], source, dest;
    MPI_Cart_coords(cartcomm, rank, 2, coords);
    int row = coords[0];
    int col = coords[1];
    int x = row + col;
    int before = x;

    MPI_Cart_shift(cartcomm, 1, -row, &source, &dest);

    MPI_Status st;
    MPI_Sendrecv_replace(&x, 1, MPI_INT, dest, 0, source, 0, cartcomm, &st);

    printf("P%d (%d, %d): %d -> %d\n", rank, row, col, before, x);

    MPI_Finalize();
    return 0;
}
