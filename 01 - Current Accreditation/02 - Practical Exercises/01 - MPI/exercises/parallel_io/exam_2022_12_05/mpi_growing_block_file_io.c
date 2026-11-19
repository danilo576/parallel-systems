#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define M 4

/*
 * Ispit 5. 12. 2022, zadatak 3.
 * Napisati MPI program za paralelni upis i čitanje binarne datoteke.
 * a) Svaki proces upisuje M(M+1)/2 proizvoljnih celih brojeva u file1.dat
 *    pomoću pojedinačnog pokazivača. Blokovi su poređani po rangu procesa.
 * b) Ponovo otvoriti datoteku, pročitati upravo upisane podatke pomoću
 *    funkcije sa eksplicitnim pomerajem i proveriti njihovu ispravnost.
 * c) Pročitane podatke efikasno upisati u novu datoteku. Najpre upisati
 *    po jedan broj iz svakog procesa, zatim po dva, pa po tri i tako
 *    dalje do M, u svakoj grupi po rangu procesa. M je celobrojna konstanta.
 */
int main(int argc, char *argv[])
{
    enum { LOCAL_COUNT = M * (M + 1) / 2 };
    int rank, process_count, read_count;
    int values[LOCAL_COUNT], read_values[LOCAL_COUNT];
    int block_lengths[M], displacements[M];
    MPI_File file;
    MPI_Status status;
    MPI_Datatype file_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);
    MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_ARE_FATAL);

    MPI_Offset offset = (MPI_Offset)rank * LOCAL_COUNT * sizeof(int);
    MPI_Offset file_size = (MPI_Offset)process_count * LOCAL_COUNT * sizeof(int);

    for (int i = 0; i < LOCAL_COUNT; i++)
        values[i] = rank * LOCAL_COUNT + i;

    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, values, LOCAL_COUNT, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_RDONLY,
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
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    printf("Process %d: read verified.\n", rank);

    /* Each round has one block per process, ordered by rank. */
    for (int k = 1; k <= M; k++)
    {
        block_lengths[k - 1] = k;
        displacements[k - 1] = process_count * (k * (k - 1) / 2) + rank * k;
    }
    MPI_Type_indexed(M, block_lengths, displacements, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);

    MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_set_view(file, 0, MPI_INT, file_type, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, read_values, LOCAL_COUNT, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
