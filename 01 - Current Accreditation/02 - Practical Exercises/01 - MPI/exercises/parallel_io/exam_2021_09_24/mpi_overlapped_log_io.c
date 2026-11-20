#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define ROUND_COUNT 5

/*
 * Ispit 24. 9. 2021, zadatak 3.
 * Napisati MPI program za paralelni upis i čitanje binarne log datoteke.
 * Svi procesi čitaju istu količinu podataka iz file1.dat. Prvi proces čita
 * sa početka datoteke, drugi čita naredni blok i tako redom.
 * 1. Pročitane podatke upisati u file2.dat tako da se redosled upisa
 *    procesa ne može unapred predvideti.
 * 2. U file3.dat procesi upisuju po 1/5 svojih podataka po krugu,
 *    od poslednjeg do prvog procesa, po round-robin principu.
 *    Upis preklopiti sa vremenski zahtevnom analizom log podataka,
 *    koju treba simulirati for petljom.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_File file;
    MPI_Offset file_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_get_size(file, &file_size);

    MPI_Offset local_size = file_size / process_count;
    if (file_size % ((MPI_Offset)ROUND_COUNT * process_count) != 0 ||
        local_size > INT_MAX)
    {
        if (rank == 0)
            fprintf(stderr, "The file size must be divisible by five times the process count, with at most %d bytes per process.\n",
                    INT_MAX);
        MPI_File_close(&file);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_count = (int)local_size;
    int chunk_count = local_count / ROUND_COUNT;
    unsigned char *buffer = malloc(local_count > 0 ? (size_t)local_count : 1);
    MPI_File_read_at_all(file, rank * local_size, buffer, local_count,
                         MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_write_shared(file, buffer, local_count, MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MCW, "file3.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);

    volatile unsigned int work = 0;
    for (int round = 0; round < ROUND_COUNT; round++)
    {
        MPI_Offset write_offset = ((MPI_Offset)round * process_count
                                + process_count - 1 - rank) * chunk_count;
        MPI_Request request;
        MPI_File_iwrite_at_all(file, write_offset, buffer + round * chunk_count,
                               chunk_count, MPI_BYTE, &request);

        /* Simulate analysis while the write is outstanding. */
        for (unsigned int i = 0; i < 1000000; i++)
            work = work * 33u + i;

        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&file);

    free(buffer);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
