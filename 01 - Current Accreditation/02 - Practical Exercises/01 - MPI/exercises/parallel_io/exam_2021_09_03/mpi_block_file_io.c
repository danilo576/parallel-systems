
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define BLOCK_SIDE 3
#define BLOCK_SIZE (BLOCK_SIDE * BLOCK_SIDE)

/*
 * Ispit 3. 9. 2021, zadatak 3.
 * Napisati MPI program za paralelni upis i čitanje binarne datoteke.
 * 1. Svaki proces upisuje devet proizvoljnih celih brojeva u file1.dat
 *    pomoću pojedinačnog pokazivača. Blokovi u datoteci idu od
 *    poslednjeg do prvog procesa.
 * 2. Ponovo otvoriti datoteku. Svaki proces čita svoje podatke pomoću
 *    eksplicitnog pomeraja i proverava njihovu ispravnost.
 * 3. Pročitane podatke paralelno upisati u novu datoteku kao matricu
 *    M × N, pri čemu M i N unosi korisnik. Svaki proces upisuje blok
 *    3 × 3, a blokovi se raspoređuju po vrstama mreže procesa.
 *    Za 6 × 9 prva vrsta blokova je P0, P1, P2, a druga P3, P4, P5.
 *    Posebno obratiti pažnju na efikasnost paralelnog upisa.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    int dimensions[2];
    int values[BLOCK_SIZE], read_values[BLOCK_SIZE], read_count;
    MPI_File file;
    MPI_Status status;
    MPI_Datatype block_type;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    if (rank == 0)
    {
        printf("Enter M and N: ");
        fflush(stdout);
        scanf("%d%d", &dimensions[0], &dimensions[1]);
    }
    MPI_Bcast(dimensions, 2, MPI_INT, 0, MCW);

    int rows = dimensions[0];
    int columns = dimensions[1];
    int block_columns = columns / BLOCK_SIDE;
    if (rows <= 0 || columns <= 0 ||
        rows % BLOCK_SIDE != 0 || columns % BLOCK_SIDE != 0 ||
        (MPI_Offset)(rows / BLOCK_SIDE) * block_columns != process_count)
    {
        if (rank == 0)
            fprintf(stderr, "M and N must be positive multiples of 3 with one block per process.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Offset offset = (MPI_Offset)(process_count - 1 - rank) * BLOCK_SIZE * sizeof(int);
    MPI_Offset file_size = (MPI_Offset)process_count * BLOCK_SIZE * sizeof(int);

    for (int i = 0; i < BLOCK_SIZE; i++)
        values[i] = rank * BLOCK_SIZE + i;

    MPI_File_open(MCW, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, values, BLOCK_SIZE, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read_at(file, offset, read_values, BLOCK_SIZE, MPI_INT, &status);
    MPI_File_close(&file);

    MPI_Get_count(&status, MPI_INT, &read_count);
    int valid = read_count == BLOCK_SIZE;
    for (int i = 0; valid && i < BLOCK_SIZE; i++)
        valid = read_values[i] == values[i];

    if (!valid)
    {
        fprintf(stderr, "Process %d: read verification failed.\n", rank);
        MPI_Abort(MCW, EXIT_FAILURE);
    }
    printf("Process %d: read verified.\n", rank);

    int block_row = rank / block_columns;
    int block_column = rank % block_columns;
    int first_row = block_row * BLOCK_SIDE;
    int first_column = block_column * BLOCK_SIDE;

    MPI_Offset element_index = (MPI_Offset)first_row * columns + first_column;
    MPI_Offset displacement = element_index * sizeof(int);

    MPI_Type_vector(BLOCK_SIDE, BLOCK_SIDE, columns, MPI_INT, &block_type);
    MPI_Type_commit(&block_type);

    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_size(file, file_size);
    MPI_File_set_view(file, displacement, MPI_INT, block_type, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, read_values, BLOCK_SIZE, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
