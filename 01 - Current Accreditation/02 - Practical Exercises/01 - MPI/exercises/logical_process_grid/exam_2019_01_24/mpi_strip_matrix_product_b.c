#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../../common/mpi_helpers.h"

#ifndef N
#define N 4
#endif
#ifndef K
#define K 2
#endif
#define MCW MPI_COMM_WORLD

/*
 * Zadatak 2, 24.01.2019, deo b.
 * Napisati MPI program za mnozenje matrica A i B dimenzija n x n.
 * Matrice inicijalizuje master proces. A je podeljena na blokove od k
 * vrsta, a B na blokove od k kolona; k je konstanta i n je deljivo sa k.
 * Postoji (n/k)^2 radnika uredjenih kao matrica. Radnici iste vrste
 * dobijaju isti blok vrsta A, a radnici iste kolone isti blok kolona B.
 * Svaki radnik racuna svoj deo C, a master prikazuje celu matricu C.
 * U delu a) svaki blok A i B poslati jednim MPI_Send pozivom sa count=1
 * i odgovarajucim izvedenim tipom podataka.
 * U delu b) slanje blokova A realizovati grupnim operacijama i
 * funkcijama za kreiranje novih komunikatora.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (N <= 0 || K <= 0 || N % K != 0 || size != 1 + (N / K) * (N / K))
    {
        if (rank == 0)
            fprintf(stderr, "Expected positive N and K, N divisible by K, and 1+(N/K)^2 processes.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int q = N / K;
    int row = rank == 0 ? -1 : (rank - 1) / q;
    int col = rank == 0 ? -1 : (rank - 1) % q;
    double a[N][N], b[N][N], c[N][N];
    double local_a[K][N], local_b[N][K], local_c[K][K];
    MPI_Datatype row_type, column_type, block_type;

    MPI_Type_contiguous(K * N, MPI_DOUBLE, &row_type);
    MPI_Type_commit(&row_type);
    MPI_Type_vector(N, K, N, MPI_DOUBLE, &column_type);
    MPI_Type_commit(&column_type);
    MPI_Type_vector(K, K, N, MPI_DOUBLE, &block_type);
    MPI_Type_commit(&block_type);

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
            {
                a[i][j] = i * N + j + 1;
                b[i][j] = i + j + 1;
            }
        print_matrix("Matrix A", N, N, &a[0][0]);
        print_matrix("Matrix B", N, N, &b[0][0]);
    }

    /* Each communicator contains one worker row followed by the master. */
    for (int r = 0; r < q; r++)
    {
        MPI_Comm row_comm;
        int color = (rank == 0 || row == r) ? 0 : MPI_UNDEFINED;
        int key = rank == 0 ? q : col;
        MPI_Comm_split(MCW, color, key, &row_comm);
        if (rank == 0)
            MPI_Bcast(&a[r * K][0], 1, row_type, q, row_comm);
        else if (row == r)
            MPI_Bcast(local_a, K * N, MPI_DOUBLE, q, row_comm);
    }

    if (rank == 0)
    {
        for (int worker = 1; worker < size; worker++)
        {
            int worker_col = (worker - 1) % q;
            MPI_Send(&b[0][worker_col * K], 1, column_type, worker, 1, MCW);
        }
        for (int worker = 1; worker < size; worker++)
        {
            int worker_row = (worker - 1) / q;
            int worker_col = (worker - 1) % q;
            MPI_Recv(&c[worker_row * K][worker_col * K], 1, block_type,
                     worker, 2, MCW, MPI_STATUS_IGNORE);
        }
        print_matrix("Matrix C", N, N, &c[0][0]);
    }
    else
    {
        MPI_Recv(local_b, N * K, MPI_DOUBLE, 0, 1, MCW, MPI_STATUS_IGNORE);
        for (int i = 0; i < K; i++)
            for (int j = 0; j < K; j++)
            {
                local_c[i][j] = 0;
                for (int t = 0; t < N; t++)
                    local_c[i][j] += local_a[i][t] * local_b[t][j];
            }
        MPI_Send(local_c, K * K, MPI_DOUBLE, 0, 2, MCW);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
