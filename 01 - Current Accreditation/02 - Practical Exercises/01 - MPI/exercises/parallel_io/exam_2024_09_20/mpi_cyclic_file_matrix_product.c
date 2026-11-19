#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MCW MPI_COMM_WORLD

enum { K = 2, M = 4, N = 2 };

/*
 * Ispit 20. 9. 2024, zadatak 1a.
 * Pomnožiti matrice A dimenzija k × m i B dimenzija m × n. Matrica A
 * je u datoteci 2009okt.dat, a B inicijalizuje master. Proces i efikasno
 * čita sve kolone j za koje važi j mod p = i. Svaki proces čita jednak
 * broj kolona matrice A i dobija celu matricu B. Svi procesi računaju
 * svoje doprinose, a rezultat se nalazi u procesu koji je pročitao
 * najmanji element matrice A. Koristiti grupne operacije, izvedene
 * tipove i MPI funkcije za rad sa datotekama. Dati primer matrica i
 * prikazati raspodelu i dobijanje rezultata.
 */
int main(int argc, char *argv[])
{
    const char *filename = "2009okt.dat";
    int rank, size;
    MPI_File file;
    MPI_Datatype file_type;
    MPI_Status status;
    int read_count;
    int b[M][N], c[K][N], local_c[K][N] = {0};
    struct {
        int value;
        int rank;
    } local_min, global_min;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (M % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "M (%d) must be divisible by the process count (%d).\n", M, size);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_width = M / size;
    int local_a[K][local_width];

    MPI_Type_vector(K * local_width, 1, size, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);
    MPI_File_open(MCW, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, (MPI_Offset)rank * sizeof(int),
                      MPI_INT, file_type, "native", MPI_INFO_NULL);
    MPI_File_read_all(file, local_a, K * local_width, MPI_INT, &status);
    MPI_File_close(&file);
    MPI_Get_count(&status, MPI_INT, &read_count);
    if (read_count != K * local_width)
    {
        fprintf(stderr, "Process %d could not read its entire share of A.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }

    if (rank == 0)
    {
        const int example_b[M][N] = {{1, 0}, {0, 1}, {1, 0}, {0, 1}};
        memcpy(b, example_b, sizeof b);
    }
    MPI_Bcast(b, M * N, MPI_INT, 0, MCW);

    local_min.value = local_a[0][0];
    local_min.rank = rank;
    for (int row = 0; row < K; row++)
        for (int inner = 0; inner < local_width; inner++)
        {
            int value = local_a[row][inner];
            int global_col = rank + inner * size;
            if (value < local_min.value)
                local_min.value = value;
            for (int col = 0; col < N; col++)
                local_c[row][col] += value * b[global_col][col];
        }

    MPI_Reduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, 0, MCW);
    MPI_Bcast(&global_min.rank, 1, MPI_INT, 0, MCW);
    MPI_Reduce(local_c, c, K * N, MPI_INT, MPI_SUM, global_min.rank, MCW);

    if (rank == global_min.rank)
    {
        printf("Process %d read the minimum of A (%d).\n", rank, local_min.value);
        puts("Matrix C:");
        for (int row = 0; row < K; row++)
            for (int col = 0; col < N; col++)
                printf("%d%c", c[row][col], col == N - 1 ? '\n' : ' ');
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
