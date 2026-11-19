#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Ispit 7. 10. 2021, zadatak 3.
 * Napisati MPI program za paralelno čitanje i upis binarne datoteke.
 * 1. Iz prethodno pripremljene datoteke file1.dat kolektivno pročitati
 *    cele brojeve tako da svaki proces dobije jednako dug uzastopni
 *    blok i da blokovi slede po redosledu rangova.
 * 2. Pročitane brojeve upisati u file2.dat prema slici za četiri procesa.
 *    Najpre svaki proces upisuje jedan broj, zatim po dva, pa po tri
 *    i tako dalje. U svakoj grupi blokovi slede po redosledu rangova.
 *    Upis preklopiti sa simuliranim vremenski zahtevnim računanjem.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_File input, output;
    MPI_Offset file_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &input);
    MPI_File_get_size(input, &file_size);

    MPI_Offset bytes_per_round = (MPI_Offset)process_count * sizeof(int);
    MPI_Offset local_size = file_size / bytes_per_round;
    if (file_size % bytes_per_round != 0 || local_size > INT_MAX)
    {
        if (rank == 0)
            fprintf(stderr, "The file must contain an equal block of up to %d integers per process.\n",
                    INT_MAX);
        MPI_File_close(&input);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_count = (int)local_size;
    int *values = malloc((size_t)local_count * sizeof(*values));
    MPI_Offset read_offset = (MPI_Offset)rank * local_count * sizeof(int);
    if (local_count > 0)
        MPI_File_read_at_all(input, read_offset, values, local_count,
                             MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&input);

    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &output);
    MPI_File_set_size(output, file_size);

    volatile unsigned int work = 0;
    for (int used = 0, round = 1; used < local_count; round++)
    {
        int remaining = local_count - used;
        int block_length = remaining < round ? remaining : round;
        MPI_Offset write_index = (MPI_Offset)process_count * used
                               + (MPI_Offset)rank * block_length;
        MPI_Request request;

        MPI_File_iwrite_at_all(output, write_index * (MPI_Offset)sizeof(int),
                               values + used, block_length, MPI_INT, &request);

        /* Simulate computation without accessing the active output buffer. */
        for (int i = 0; i < 1000000; i++)
            work = work * 33u + (unsigned int)i;

        MPI_Wait(&request, MPI_STATUS_IGNORE);
        used += block_length;
    }

    MPI_File_close(&output);
    free(values);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
