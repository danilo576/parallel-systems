#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define M 6

/*
 * Ispit 7. 7. 2022, zadatak 3.
 * Napisati MPI program za paralelni upis i čitanje binarne datoteke.
 * a) Prvi proces upisuje cele brojeve od 0 do M-1 od početka datoteke,
 *    drugi upisuje brojeve od M do 2M-1 sa pomerajem M i tako dalje.
 *    Koristiti pojedinačne pokazivače na datoteku.
 * b) Ponovo otvoriti datoteku. Svaki proces čita podatke koje je upisao
 *    koristeći funkciju sa eksplicitnim pomerajem i proverava čitanje.
 * c) Svaki proces upisuje pročitane podatke u novu datoteku. Za M = 6
 *    i tri procesa redosled je 0 1 6 7 12 13 2 3 8 9 14 15 4 5 10 11 16 17.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    int values[M], read_values[M], read_count;
    MPI_File file;
    MPI_Status status;
    MPI_Datatype file_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    if (M % process_count != 0)
    {
        if (rank == 0)
            fprintf(stderr, "The process count must divide M (%d).\n", M);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Offset offset = (MPI_Offset)rank * M * sizeof(int);
    MPI_Offset file_size = (MPI_Offset)process_count * M * sizeof(int);

    for (int i = 0; i < M; i++)
        values[i] = rank * M + i;

    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, values, M, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read_at(file, offset, read_values, M, MPI_INT, &status);
    MPI_File_close(&file);

    MPI_Get_count(&status, MPI_INT, &read_count);
    int valid = read_count == M;
    for (int i = 0; valid && i < M; i++)
        valid = read_values[i] == values[i];

    if (!valid)
    {
        fprintf(stderr, "Process %d: read verification failed.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    printf("Process %d: read verified.\n", rank);

    int block_size = M / process_count;
    MPI_Type_vector(process_count, block_size, M, MPI_INT, &file_type);
    MPI_Type_commit(&file_type);

    MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_set_view(file, (MPI_Offset)rank * block_size * sizeof(int),
                      MPI_INT, file_type, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, read_values, M, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
