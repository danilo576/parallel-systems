#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Ispit 2. 7. 2021, zadatak 3.
 * Napisati MPI program za paralelni upis i čitanje binarne datoteke.
 * Svaki proces upisuje 105 proizvoljnih celih brojeva u file1.dat
 * pomoću pojedinačnog pokazivača. Podaci su poređani od poslednjeg
 * do prvog procesa. Ponovo otvoriti datoteku, pročitati sopstvene
 * podatke pomoću funkcije sa eksplicitnim pomerajem i proveriti ih.
 * Pročitane podatke efikasno upisati u novu datoteku prema slici:
 * po jedan broj iz svakog procesa, zatim po dva, po tri i tako dalje,
 * u svakoj grupi od prvog do poslednjeg procesa.
 */
int main(int argc, char *argv[])
{
    enum { LOCAL_COUNT = 105 };
    int rank, process_count, read_count;
    int values[LOCAL_COUNT], read_values[LOCAL_COUNT];
    MPI_File file;
    MPI_Status status;
    MPI_Datatype file_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    MPI_Offset offset = (MPI_Offset)(process_count - 1 - rank) * LOCAL_COUNT * sizeof(int);
    MPI_Offset file_size = (MPI_Offset)process_count * LOCAL_COUNT * sizeof(int);

    for (int i = 0; i < LOCAL_COUNT; i++)
        values[i] = rank * LOCAL_COUNT + i;

    MPI_File_open(MCW, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, values, LOCAL_COUNT, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_read_at_all(file, offset, read_values, LOCAL_COUNT, MPI_INT, &status);
    MPI_File_close(&file);

    MPI_Get_count(&status, MPI_INT, &read_count);
    int valid = read_count == LOCAL_COUNT;
    for (int i = 0; valid && i < LOCAL_COUNT; i++)
        valid = read_values[i] == values[i];

    if (!valid)
    {
        fprintf(stderr, "Process %d: read verification failed.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }
    printf("Process %d: read verified.\n", rank);

    int block_lengths[LOCAL_COUNT], displacements[LOCAL_COUNT];
    int rounds = 0;
    int used = 0;
    while (used < LOCAL_COUNT)
    {
        int length = rounds + 1;
        block_lengths[rounds] = length;
        displacements[rounds] = process_count * used + rank * length;
        used += length;
        rounds++;
    }
    MPI_Type_indexed(rounds, block_lengths, displacements, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);

    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_set_view(file, 0, MPI_INT, file_type, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, read_values, LOCAL_COUNT, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
