#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * 25.04.2023. — Zadatak 1
 *
 * Napisati MPI program za razmenu poruka između procesa u dvodimenzionalnoj
 * kartezijanskoj topologiji dimenzija m × n. Procesi razmenjuju celobrojni
 * niz od tri parametra sa susedima iz iste vrste u kolonama iste parnosti.
 * Parametre inicijalizovati proizvoljno na osnovu koordinata procesa.
 * Slanje parametara svakom procesu obaviti jednom naredbom koja šalje
 * jedan primerak izvedenog tipa. Svaki proces uvećava primljene podatke
 * za identifikator svog levog suseda. Procesi u neparnim kolonama ispisuju
 * vrednosti niza pre i posle slanja. Ilustrovati raspored procesa za
 * izabrane vrednosti m i n. Broj kolona je paran.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (argc != 3) {
        if (rank == 0)
            fprintf(stderr, "Usage: %s <rows> <even columns>\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    char *row_end, *column_end;
    long rows = strtol(argv[1], &row_end, 10);
    long columns = strtol(argv[2], &column_end, 10);
    if (*row_end != '\0' || *column_end != '\0' ||
        rows < 1 || columns < 2 || columns > size ||
        columns % 2 != 0 || size % columns != 0 || rows != size / columns) {
        if (rank == 0)
            fprintf(stderr, "Use positive dimensions with an even column count "
                            "and rows * columns equal to the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int dims[2] = {(int)rows, (int)columns};
    int periods[2] = {0, 1};
    MPI_Comm cart_comm;
    MPI_Cart_create(MCW, 2, dims, periods, 0, &cart_comm);

    int coords[2], left, right;
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    MPI_Cart_shift(cart_comm, 1, 2, &left, &right);

    int values[3] = {coords[0], coords[1], coords[0] + coords[1]};
    MPI_Datatype triplet_type;
    MPI_Type_contiguous(3, MPI_INT, &triplet_type);
    MPI_Type_commit(&triplet_type);

    if (coords[1] % 2 != 0)
        printf("P%d (%d, %d): before = [%d, %d, %d]\n",
               rank, coords[0], coords[1], values[0], values[1], values[2]);

    MPI_Sendrecv_replace(values, 1, triplet_type, right, 0,
                         left, 0, cart_comm, MPI_STATUS_IGNORE);

    for (int i = 0; i < 3; ++i)
        values[i] += left;

    if (coords[1] % 2 != 0)
        printf("P%d (%d, %d): after  = [%d, %d, %d]\n",
               rank, coords[0], coords[1], values[0], values[1], values[2]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
