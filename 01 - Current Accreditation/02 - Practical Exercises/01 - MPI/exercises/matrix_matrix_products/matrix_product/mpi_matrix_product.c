#include <stdio.h>
#include <mpi.h>

#define MCW MPI_COMM_WORLD
#define N 3
#define K 6
#define Q 2

/*
 * Napisati MPI program koji realizuje mnozenje matrice A dimenzija n x k
 * i matrice B dimenzija k x n, cime se dobija i prikazuje rezultujuca
 * matrica C. Takodje, program pronalazi i prikazuje proizvod elemenata
 * svake kolone matrice B. Izracunavanje se obavlja tako sto master proces
 * salje svakom procesu po q kolona matrice A (q je zadata konstanta)
 * i po q vrsta matrice B. Svi elementi kolona matrice A koji se salju
 * svakom procesu salju se odjednom. Svi procesi ucestvuju u izracunavanjima
 * potrebnim za generisanje rezultata programa. Zadatak resiti primenom
 * grupnih operacija, osim za slanje kolona matrice A za koje se koristi
 * Point-to-Point operacija. Ne koristiti izvedene tipove podataka.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    double a[N][K], b[K][N], c[N][N];
    double local_a[N][Q], local_b[Q][N];
    double local_c[N][N] = {0};
    double local_product[N], product[N];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K != Q * size)
    {
        if (rank == 0)
            printf("The number of processes must satisfy K = Q * size.\n");
        MPI_Finalize();
        return 0;
    }

    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < K; j++)
                a[i][j] = i + j + 1;

        for (int i = 0; i < K; i++)
            for (int j = 0; j < N; j++)
                b[i][j] = (i + j) % 3 + 1;
    }

    if (rank == 0)
    {
        for (int p = 1; p < size; p++)
        {
            /* Pack all Q columns into one contiguous buffer. */
            for (int i = 0; i < N; i++)
                for (int j = 0; j < Q; j++)
                    local_a[i][j] = a[i][p * Q + j];

            MPI_Send(local_a, N * Q, MPI_DOUBLE, p, 0, MCW);
        }

        for (int i = 0; i < N; i++)
            for (int j = 0; j < Q; j++)
                local_a[i][j] = a[i][j];
    }
    else
        MPI_Recv(local_a, N * Q, MPI_DOUBLE, 0, 0, MCW,
                 MPI_STATUS_IGNORE);

    MPI_Scatter(b, Q * N, MPI_DOUBLE, local_b, Q * N, MPI_DOUBLE,
                0, MCW);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int t = 0; t < Q; t++)
                local_c[i][j] += local_a[i][t] * local_b[t][j];

    for (int i = 0; i < N; i++) 
    {
        local_product[i] = 1;
        for (int j = 0; j < Q; j++)
            local_product[i] *= local_b[j][i];
    }

    MPI_Reduce(local_c, c, N * N, MPI_DOUBLE, MPI_SUM, 0, MCW);
    MPI_Reduce(local_product, product, N, MPI_DOUBLE, MPI_PROD, 0, MCW);

    if (rank == 0)
    {
        printf("Matrix C:\n");
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
                printf("%g ", c[i][j]);
            printf("\n");
        }

        printf("Column products of B:\n");
        for (int j = 0; j < N; j++)
            printf("%g ", product[j]);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}
