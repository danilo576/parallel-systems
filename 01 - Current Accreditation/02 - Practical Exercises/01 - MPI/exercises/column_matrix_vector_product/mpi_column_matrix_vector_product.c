#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define K 3
#define M 6
#define MCW MPI_COMM_WORLD

/*
 * 7. a) Napisati MPI program koji racuna proizvod matrice A dimenzija k x m
 * i vektora B duzine m. Matricu A ucitava master proces iz fajla
 * "april23.dat" u celosti, dok je vektor B inicijalizovan u svim procesima.
 * Svaki proces od master procesa dobija jednak broj sukcesivnih kolona
 * matrice A (broj kolona je deljiv brojem procesa). Svi procesi ucestvuju
 * u izracunavanju rezultata, koji se na kraju prikazuje iz procesa sa
 * najvecim identifikatorom. Zadatak resiti primenom grupnih operacija
 * i izvedenih tipova podataka. Dati primer matrice A i vektora B i
 * ilustrovati podelu podataka po procesima i dobijanje rezultata.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    double a[K][M], b[M], local_result[K] = {0}, result[K];
    MPI_Datatype column_block, resized_block;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (M % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "M (%d) must be divisible by the number of processes (%d).\n", M, size);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_cols = M / size;
    double local_a[K][local_cols];

    for (int j = 0; j < M; j++)
        b[j] = j + 1;

    if (rank == 0)
    {
        MPI_File file;
        MPI_Status status;
        int count;

        if (MPI_File_open(MPI_COMM_SELF, "april23.dat", MPI_MODE_RDONLY,
                          MPI_INFO_NULL, &file) != MPI_SUCCESS)
        {
            fprintf(stderr, "Cannot open april23.dat.\n");
            MPI_Abort(MCW, EXIT_FAILURE);
            return EXIT_FAILURE;
        }

        if (MPI_File_read(file, a, K * M, MPI_DOUBLE, &status) != MPI_SUCCESS)
        {
            fprintf(stderr, "Cannot read matrix from april23.dat.\n");
            MPI_Abort(MCW, EXIT_FAILURE);
            return EXIT_FAILURE;
        }
        MPI_Get_count(&status, MPI_DOUBLE, &count);
        MPI_File_close(&file);

        if (count != K * M)
        {
            fprintf(stderr, "Not enough matrix elements in april23.dat.\n");
            MPI_Abort(MCW, EXIT_FAILURE);
            return EXIT_FAILURE;
        }
    }

    MPI_Type_vector(K, local_cols, M, MPI_DOUBLE, &column_block);
    MPI_Type_create_resized(column_block, 0,
                            (MPI_Aint)local_cols * sizeof(double), &resized_block);
    MPI_Type_commit(&resized_block);

    MPI_Scatter(a, 1, resized_block, local_a, K * local_cols,
                MPI_DOUBLE, 0, MCW);

    for (int i = 0; i < K; i++)
        for (int j = 0; j < local_cols; j++)
            local_result[i] += local_a[i][j] * b[rank * local_cols + j];

    MPI_Reduce(local_result, result, K, MPI_DOUBLE, MPI_SUM, size - 1, MCW);

    if (rank == size - 1)
    {
        printf("Process %d: result C =", rank);
        for (int i = 0; i < K; i++)
            printf(" %g", result[i]);
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
