#include <mpi.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define ROUND_COUNT 10

/*
 * Ispit 30. 3. 2022, zadatak 3.
 * Napisati MPI program za paralelno čitanje i upis binarnih log podataka.
 * Iz datoteke file1.dat svi procesi čitaju jednako duge uzastopne blokove
 * po redosledu rangova, počevši od prvog procesa na početku datoteke.
 * a) Pročitane podatke upisati u file2.dat tako da se redosled upisa
 *    procesa ne može unapred predvideti.
 * b) U file3.dat procesi upisuju po 1/10 svojih podataka u svakom krugu,
 *    u redosledu od poslednjeg do prvog procesa (round-robin).
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_File file;
    MPI_Offset file_size;
    MPI_Comm reverse_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_get_size(file, &file_size);

    MPI_Offset local_size = file_size / process_count;
    int local_count = (int)local_size;
    int chunk_count = local_count / ROUND_COUNT;
    unsigned char *buffer = malloc(local_count > 0 ? (size_t)local_count : 1);
    MPI_Offset read_offset = rank * local_size;
    MPI_File_read_at_all(file, read_offset, buffer, local_count,
                         MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_write_shared(file, buffer, local_count, MPI_BYTE, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Comm_split(MCW, 0, process_count - 1 - rank, &reverse_comm);
    MPI_File_open(reverse_comm, "file3.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    for (int round = 0; round < ROUND_COUNT; round++)
    {
        MPI_File_write_ordered(file, buffer + round * chunk_count,
                               chunk_count, MPI_BYTE, MPI_STATUS_IGNORE);
    }
    MPI_File_close(&file);

    free(buffer);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
