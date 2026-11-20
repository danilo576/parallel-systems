#include <mpi.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD

/*
 * 20.07.2026, zadatak 7b
 *
 * Napisati MPI program koji računa proizvod vrednosti parametra X
 * iz procesa P1, P2, ..., Pn (svih sem P0) komunikatora MPI_COMM_WORLD.
 * Proizvod prikazati iz procesa P5. Zadatak rešiti korišćenjem grupnih
 * operacija i komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    int excluded_rank = 0;
    MPI_Group world_group, product_group;
    MPI_Comm product_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (size < 6) {
        if (rank == 0)
            printf("Run with at least 6 processes.\n");
        MPI_Finalize();
        return 1;
    }

    MPI_Comm_group(MCW, &world_group);
    MPI_Group_excl(world_group, 1, &excluded_rank, &product_group);
    MPI_Comm_create(MCW, product_group, &product_comm);

    if (rank != 0) {
        long long x = rank;
        long long product;

        MPI_Reduce(&x, &product, 1, MPI_LONG_LONG_INT, MPI_PROD, 4, product_comm);

        if (rank == 5)
            printf("P5: product = %lld\n", product);
    }

    MPI_Finalize();
    return 0;
}
