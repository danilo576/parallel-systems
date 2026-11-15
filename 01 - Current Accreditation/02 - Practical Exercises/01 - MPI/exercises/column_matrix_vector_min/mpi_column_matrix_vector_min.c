#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../common/mpi_helpers.h"

#define M 4
#define K 8
#define S 2
#define MCW MPI_COMM_WORLD

/*
 * Napisati MPI program koji realizuje mnozenje matrice A dimenzija m x k
 * i vektora b duzine k, cime se dobija i prikazuje rezultujuci vektor c.
 * Takodje, program pronalazi i prikazuje minimalnu vrednost elemenata svake
 * vrste matrice A. Elementi matrice A se inicijalizuju u root procesu, dok
 * su elementi vektora b inicijalizovani u svim procesima.
 * Izracunavanje se obavlja tako sto master proces salje svakom procesu po
 * s susednih kolona matrice A (s je zadata konstanta). Svi elementi kolona
 * salju se odjednom i direktno iz matrice A. Minimalna vrednost elemenata
 * svake vrste matrice A pronalazi se nakon distribuiranja vrednosti matrice A.
 * Svi procesi ucestvuju u izracunavanjima potrebnim za generisanje rezultata.
 * Zadatak resiti primenom grupnih operacija.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    double a[M][K], b[K], c[M];
    double local_a[M][S];
    double local_c[M] = {0};
    MPI_Datatype column_block;
    double local_min[M], global_min[M];    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (K != S * size)
    {
        if (rank == 0)
            fprintf(stderr, "Expected K = S * processes (K=%d, S=%d).\n", K, S);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    for (int j = 0; j < K; j++)
        b[j] = j + 1;

    MPI_Type_vector(M, S, K, MPI_DOUBLE, &column_block);
    MPI_Type_commit(&column_block);

    if (rank == 0)
    {
        const double input[M][K] = {
            {5, 2, 8, 1, 9, 4, 7, 3},
            {6, 9, -3, 8, 5, 2, 4, 7},
            {9, 6, 8, 7, 3, 5, -4, 2},
            {8, 7, 6, 5, -2, 9, 4, 3}
        };
        for (int i = 0; i < M; i++)
            for (int j = 0; j < K; j++)
                a[i][j] = input[i][j];

        for (int i = 0; i < M; i++)
            for (int j = 0; j < S; j++)
                local_a[i][j] = a[i][j];

        for (int p = 1; p < size; p++)
            MPI_Send(&a[0][p * S], 1, column_block, p, 0, MCW);
    } 
    else 
    {
        MPI_Recv(&local_a[0][0], M * S, MPI_DOUBLE, 0, 0, MCW, MPI_STATUS_IGNORE);
    }
    
    for (int i = 0; i < M; i++)
        for (int j = 0; j < S; j++)
            local_c[i] += local_a[i][j] * b[rank * S + j];

    for (int i = 0; i < M; i++) 
    {
        local_min[i] = local_a[i][0];
        for (int j = 1; j < S; j++) 
            if (local_a[i][j] < local_min[i])
                local_min[i] = local_a[i][j];
    }

    MPI_Reduce(local_c, c, M, MPI_DOUBLE, MPI_SUM, 0, MCW);
    MPI_Reduce(local_min, global_min, M, MPI_DOUBLE, MPI_MIN, 0, MCW);

    if (rank == 0)
    {
        print_matrix("Vector C:", M, 1, c);
        print_matrix("Min A per rows:", M, 1, global_min);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
