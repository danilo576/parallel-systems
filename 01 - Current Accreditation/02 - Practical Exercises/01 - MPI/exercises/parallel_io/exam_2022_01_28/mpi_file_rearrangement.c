#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Ispit 28.01.2022, zadatak 3.
 * Datoteka file1.dat sadrži 10 MB zapisa o mrežnom saobraćaju.
 * Napisati MPI program za paralelno čitanje i upis radi pripreme podataka
 * za vizuelizaciju. Svaki proces čita istu količinu podataka, pri čemu
 * se pozicija sa koje pojedini proces čita ne može unapred predvideti.
 * Svaki proces deli pročitane podatke na dva jednaka dela. U file2.dat
 * najpre se upisuju prve polovine svih procesa po rangu, zatim druge
 * polovine istim redom. Obratiti pažnju na efikasnost paralelnog upisa.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_File input, output;
    MPI_Offset file_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);
    MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_ARE_FATAL);

    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &input);
    MPI_File_get_size(input, &file_size);

    if (file_size == 0 || file_size > INT_MAX ||
        file_size % (2 * (MPI_Offset)process_count) != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Input size must be positive, at most INT_MAX bytes, "
                            "and divisible by twice the process count.\n");
        MPI_File_close(&input);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_size = (int)(file_size / process_count);
    int half_size = local_size / 2;
    unsigned char *buffer = malloc(local_size);

    MPI_File_read_shared(input, buffer, local_size, MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&input);

    MPI_Datatype file_type;
    MPI_Type_vector(2, half_size, (int)(file_size / 2), MPI_BYTE, &file_type);
    MPI_Type_commit(&file_type);

    MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &output);
    MPI_File_set_size(output, file_size);
    MPI_File_set_view(output, (MPI_Offset)rank * half_size, MPI_BYTE, file_type,
                      "native", MPI_INFO_NULL);
    MPI_File_write_all(output, buffer, local_size, MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&output);

    free(buffer);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
