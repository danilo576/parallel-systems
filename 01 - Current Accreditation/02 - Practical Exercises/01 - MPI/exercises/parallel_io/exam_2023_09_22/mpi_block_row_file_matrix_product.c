#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Ispit 22. 9. 2023, zadatak 1.
 * Napisati MPI program koji množi matrice A (k × m) i B (m × n).
 * Matrica A je smeštena po kolonama u fajlu matA.dat, a matricu B
 * učitava master. Svaki od p procesa čita k/p uzastopnih vrsta A
 * (k je deljivo sa p): P0 čita prvih k/p, P1 sledećih k/p itd.
 * Master distribuira celu matricu B ostalim procesima. Svaki proces
 * obavlja odgovarajuća izračunavanja i učestvuje u generisanju rezultata.
 * Matrica C treba da se nađe u procesu sa minimalnom sumom učitanih
 * elemenata matrice A.
 */
int main(int argc, char *argv[])
{
    int rank, size, dimensions[3];
    MPI_File file;
    MPI_Datatype file_type;
    MPI_Status status;
    int elements_read;
    struct {
        int sum;
        int rank;
    } local, minimum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (rank == 0)
        scanf("%d%d%d", &dimensions[0], &dimensions[1], &dimensions[2]);
    MPI_Bcast(dimensions, 3, MPI_INT, 0, MCW);

    int k = dimensions[0], m = dimensions[1], n = dimensions[2];
    if (k <= 0 || m <= 0 || n <= 0 || k % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Dimensions must be positive and k must be divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int rows = k / size;
    int *local_a = malloc((size_t)rows * m * sizeof *local_a);
    int *b = malloc((size_t)m * n * sizeof *b);
    int *local_c = malloc((size_t)rows * n * sizeof *local_c);

    MPI_Type_vector(m, rows, k, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);
    if (MPI_File_open(MCW, "matA.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file) != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Could not open matA.dat.\n");
        MPI_Abort(MCW, EXIT_FAILURE);
    }

    MPI_Offset displacement = (MPI_Offset)rank * rows * sizeof(int);
    MPI_File_set_view(file, displacement, MPI_INT, file_type, "native", MPI_INFO_NULL);
    MPI_File_read_all(file, local_a, rows * m, MPI_INT, &status);
    MPI_File_close(&file);
    MPI_Get_count(&status, MPI_INT, &elements_read);
    if (elements_read != rows * m)
    {
        fprintf(stderr, "Process %d could not read its entire block of A.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }

    if (rank == 0)
        for (int i = 0; i < m * n; i++)
            scanf("%d", &b[i]);
    MPI_Bcast(b, m * n, MPI_INT, 0, MCW);

    local.sum = 0;
    local.rank = rank;
    for (int i = 0; i < rows * m; i++)
        local.sum += local_a[i];

    for (int row = 0; row < rows; row++)
        for (int col = 0; col < n; col++)
        {
            int value = 0;
            for (int inner = 0; inner < m; inner++)
                value += local_a[inner * rows + row] * b[inner * n + col];
            local_c[row * n + col] = value;
        }

    MPI_Reduce(&local, &minimum, 1, MPI_2INT, MPI_MINLOC, 0, MCW);
    MPI_Bcast(&minimum.rank, 1, MPI_INT, 0, MCW);

    int *c = NULL;
    if (rank == minimum.rank)
        c = malloc((size_t)k * n * sizeof *c);
    MPI_Gather(local_c, rows * n, MPI_INT, c, rows * n, MPI_INT, minimum.rank, MCW);

    if (rank == minimum.rank)
    {
        printf("Process %d has the minimum sum of A (%d).\n", rank, local.sum);
        puts("Matrix C:");
        for (int row = 0; row < k; row++)
            for (int col = 0; col < n; col++)
                printf("%d%c", c[row * n + col], col == n - 1 ? '\n' : ' ');
    }

    free(c);
    free(local_c);
    free(b);
    free(local_a);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
