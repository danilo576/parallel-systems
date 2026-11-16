#include <mpi.h>

/* Standalone helper program that generates the input file for the matrix-vector product. */
/* Generisati binarni fajl april23.dat sa primerom matrice dimenzija 3 x 6. */
int main(int argc, char *argv[])
{
    int rank;
    double a[3][6];
    MPI_File file;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 6; j++)
                a[i][j] = i * 6 + j + 1;

        if (MPI_File_open(MPI_COMM_SELF, "april23.dat",
                          MPI_MODE_CREATE | MPI_MODE_WRONLY,
                          MPI_INFO_NULL, &file) != MPI_SUCCESS)
        {
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        MPI_File_set_size(file, 0); // Remove previous content from the file if it exists.
        MPI_File_write(file, a, 18, MPI_DOUBLE, MPI_STATUS_IGNORE);
        MPI_File_close(&file);
    }

    MPI_Finalize();
    return 0;
}
