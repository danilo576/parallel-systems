#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifndef N
#define N 4
#endif

#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2b, 28.01.2022.
 * Napisati MPI program za množenje kvadratnih matrica A i B reda n.
 * Master inicijalizuje A i B. Procesi čine mrežu q × q, gde je q² = p.
 * Svi procesi u vrsti r dobijaju vrste r, r+q, ..., r+n−q matrice A.
 * Svi procesi u koloni c dobijaju kolone c, c+q, ..., c+n−q matrice B.
 * Svaki blok jedne matrice prenosi se odjednom. Procesi računaju svoje
 * blokove proizvoda C, koji se prikazuje u master procesu.
 * Raspodelu A i B realizovati point-to-point operacijama, uz korišćenje
 * samo podrazumevanog komunikatora.
 */
int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int q = 1;
    while ((long long)q * q < size)
        q++;
    if ((long long)q * q != size || N <= 0 || N % q != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected p = q*q and positive N divisible by q.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const int n = N;
    const int s = n / q;
    double a[N][N], b[N][N], c[N][N];
    double local_a[s][N], local_b[N][s];

    if (rank == 0)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
            {
                a[i][j] = i + 1;
                b[i][j] = j + 1;
            }

    MPI_Datatype a_strip, b_strip;
    MPI_Type_vector(s, n, q * n, MPI_DOUBLE, &a_strip);
    MPI_Type_vector(n * s, 1, q, MPI_DOUBLE, &b_strip);
    MPI_Type_commit(&a_strip);
    MPI_Type_commit(&b_strip);

    if (rank == 0)
    {
        for (int dest = 1; dest < size; dest++)
        {
            MPI_Send(&a[dest / q][0], 1, a_strip, dest, 0, MCW);
            MPI_Send(&b[0][dest % q], 1, b_strip, dest, 1, MCW);
        }
        for (int i = 0; i < s; i++)
            for (int j = 0; j < n; j++)
            {
                local_a[i][j] = a[q * i][j];
                local_b[j][i] = b[j][q * i];
            }
    }
    else
    {
        MPI_Recv(local_a, s * n, MPI_DOUBLE, 0, 0, MCW, MPI_STATUS_IGNORE);
        MPI_Recv(local_b, n * s, MPI_DOUBLE, 0, 1, MCW, MPI_STATUS_IGNORE);
    }

    double local_c[s][s];
    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++)
        {
            double sum = 0;
            for (int t = 0; t < n; t++)
                sum += local_a[i][t] * local_b[t][j];
            local_c[i][j] = sum;
        }

    double blocks[rank == 0 ? size : 1][s][s];
    MPI_Gather(local_c, s * s, MPI_DOUBLE, blocks, s * s, MPI_DOUBLE, 0, MCW);

    if (rank == 0)
    {
        /* Fill C in global row and column order. */
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
            {
                int owner = (i % q) * q + j % q;
                c[i][j] = blocks[owner][i / q][j / q];
            }

        puts("Matrix C");
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                printf("%g%c", c[i][j], j == n - 1 ? '\n' : ' ');
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
