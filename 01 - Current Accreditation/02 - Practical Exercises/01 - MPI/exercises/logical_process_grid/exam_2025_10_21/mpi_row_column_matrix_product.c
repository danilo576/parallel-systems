#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 2
#define MCW MPI_COMM_WORLD

/*
 * Zadatak, 21.10.2025.
 * Napisati MPI program koji mnozi matricu A dimenzija n x n i matricu B
 * dimenzija n x n, cime se dobija matrica C dimenzija n x n. Matrice A i B
 * inicijalizuju se u master procesu. Broj procesa je p, a procesi su
 * uredjeni kao matrica q x q, gde je q*q = p i q = n.
 * Master distribuira odgovarajuce vrste A i kolone B svim procesima.
 * Procesi u i-toj vrsti mreze sadrze i-tu vrstu A, a procesi u j-toj
 * koloni mreze sadrze j-tu kolonu B. Svaki proces obavlja odgovarajuca
 * izracunavanja i ucestvuje u generisanju rezultata koji prikazuje master.
 * Slanje dela svake matrice svakom procesu obaviti odjednom, direktno iz
 * matrice, koristeci point-to-point operacije. Za ostatak izracunavanja
 * koristiti grupne operacije. Nije dozvoljeno kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    double a[N][N], b[N][N], c[N][N];
    double local_a[N], local_b[N], local_c = 0;
    MPI_Datatype column_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (N <= 0 || size != N * N)
    {
        if (rank == 0)
            fprintf(stderr, "Expected positive N, and exactly N*N processes.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Type_vector(N, 1, N, MPI_DOUBLE, &column_type);
    MPI_Type_commit(&column_type);

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
            for (int k = 0; k < N; k++)
                a[i][k] = i * N + k + 1;
        for (int k = 0; k < N; k++)
            for (int j = 0; j < N; j++)
                b[k][j] = k * N + j + 1;

        print_matrix("Matrix A", N, N, &a[0][0]);
        print_matrix("Matrix B", N, N, &b[0][0]);

        /* The master keeps row 0 of A and column 0 of B. */
        for (int k = 0; k < N; k++)
        {
            local_a[k] = a[0][k];
            local_b[k] = b[k][0];
        }
        for (int dest = 1; dest < size; dest++)
        {
            int row = dest / N;
            int col = dest % N;
            MPI_Send(a[row], N, MPI_DOUBLE, dest, 0, MCW);
            MPI_Send(&b[0][col], 1, column_type, dest, 1, MCW);
        }
    }
    else
    {
        MPI_Recv(local_a, N, MPI_DOUBLE, 0, 0, MCW, MPI_STATUS_IGNORE);
        MPI_Recv(local_b, N, MPI_DOUBLE, 0, 1, MCW, MPI_STATUS_IGNORE);
    }

    for (int k = 0; k < N; k++)
        local_c += local_a[k] * local_b[k];

    MPI_Gather(&local_c, 1, MPI_DOUBLE, c, 1, MPI_DOUBLE, 0, MCW);

    if (rank == 0)
        print_matrix("Matrix C", N, N, &c[0][0]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
