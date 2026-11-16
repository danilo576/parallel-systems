#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#define N 8
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 3, 12.06.2019.
 * Napisati MPI program koji realizuje množenje matrice A dimenzija n x n
 * i vektora b dužine n, čime se dobija rezultujući vektor c dužine n.
 * Matrica A i vektor b se inicijalizuju u master procesu. Broj procesa
 * je p i uređeni su kao matrica q x q (q*q = p). Matrica A je podeljena
 * u podmatrice dimenzija k x k (k = n/q), a master distribuira blokove
 * po procesima kao na slici za n = 8 i p = 16. Proces P(r*q+s) dobija
 * vrste r*k do (r+1)*k-1 i kolone s*k do (s+1)*k-1.
 * Vektor b se distribuira u delovima od po n/q elemenata tako da procesi
 * u prvoj koloni sadrže prvih n/q elemenata, u drugoj sledećih n/q itd.
 * Na osnovu primljenih podataka svaki proces obavlja odgovarajuća
 * izračunavanja i učestvuje u generisanju rezultata koji se prikazuje
 * u master procesu. Slanje podmatrice A svakom procesu obaviti jednom
 * naredbom MPI_Send kojom se šalje samo jedan izvedeni tip podataka.
 * Slanje blokova vektora b i generisanje rezultata implementirati
 * korišćenjem grupnih operacija i funkcija za kreiranje komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size, q = 1;
    double a[N][N], b[N], c[N];
    MPI_Comm row_comm, col_comm;
    MPI_Datatype block_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    while (q < size / q)
        q++;
    if (q * q != size || N <= 0 || N % q != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected p = q*q and positive N divisible by q.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int row = rank / q, col = rank % q;
    int k = N / q;
    double local_a[k][k], local_b[k];
    double local_c[k], row_c[k];

    MPI_Comm_split(MCW, row, col, &row_comm);
    MPI_Comm_split(MCW, col, row, &col_comm);

    if (rank == 0)
    {
        for (int j = 0; j < N; j++)
            b[j] = j + 1;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                a[i][j] = i * N + j + 1;

        print_matrix("Initial matrix A", N, N, &a[0][0]);
        print_matrix("Initial vector B", N, 1, b);
    }

    MPI_Type_vector(k, k, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    if (rank == 0)
    {
        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
                local_a[i][j] = a[i][j];
        for (int dest = 1; dest < size; dest++)
            MPI_Send(&a[(dest / q) * k][(dest % q) * k],
                     1, block_type, dest, 0, MCW);
    }
    else
        MPI_Recv(local_a, k * k, MPI_DOUBLE, 0, 0,
                 MCW, MPI_STATUS_IGNORE);

    if (row == 0)
        MPI_Scatter(b, k, MPI_DOUBLE, local_b, k, MPI_DOUBLE, 0, row_comm);
    MPI_Bcast(local_b, k, MPI_DOUBLE, 0, col_comm);

    for (int i = 0; i < k; i++)
    {
        local_c[i] = 0;
        for (int j = 0; j < k; j++)
            local_c[i] += local_a[i][j] * local_b[j];
    }

    MPI_Reduce(local_c, row_c, k, MPI_DOUBLE, MPI_SUM, 0, row_comm);
    if (col == 0)
        MPI_Gather(row_c, k, MPI_DOUBLE, c, k, MPI_DOUBLE, 0, col_comm);

    if (rank == 0)
    {
        printf("Vector C:");
        for (int i = 0; i < N; i++)
            printf(" %g", c[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
