#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 25.04.2023. — Zadatak 2
 *
 * Napisati MPI program kojim se u dvodimenzionalnoj Kartezijanskoj
 * topologiji dimenzija n × m razmenjuju poruke između procesa. Procesi
 * razmenjuju parametar K sa susedima iz iste vrste u kolonama iste
 * parnosti. Parametar K inicijalizovati proizvoljno. Svaki proces uvećava
 * primljeni podatak vrednošću svog identifikatora. Procesi u parnim
 * kolonama štampaju K pre i posle razmene.
 *
 * Ilustrovati raspored procesa za izabrane vrednosti n i m.
 * Smatrati da topologija ima paran broj kolona.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

    int coords[2], source, destination;
    MPI_Cart_coords(cart_comm, rank, 2, coords);
    MPI_Cart_shift(cart_comm, 1, 2, &source, &destination);

    long long k = rank;
    if (coords[1] % 2 == 0)
        printf("P%d (%d, %d): K before = %lld\n", rank, coords[0], coords[1], k);

    MPI_Sendrecv_replace(&k, 1, MPI_LONG_LONG_INT, destination, 0,
                         source, 0, cart_comm, MPI_STATUS_IGNORE);
    k += rank;

    if (coords[1] % 2 == 0)
        printf("P%d (%d, %d): K after  = %lld\n", rank, coords[0], coords[1], k);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
