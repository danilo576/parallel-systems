#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * Ispit 22.10.2021, zadatak 3.
 * Svaki proces upisuje tekst proizvoljnog sadržaja dužine N u file1.dat
 * pomoću pojedinačnog pokazivača. Blokovi u datoteci idu od poslednjeg
 * do prvog procesa.
 * Ponovo otvoriti datoteku. Svaki proces čita N znakova tako da se
 * unapred ne zna koji će proces dobiti koji blok. Obezbediti konzistentnost.
 * Pročitane podatke efikasno upisati u file2.dat kao matricu N × N.
 * Svaki proces upisuje po jednu kolonu, redom po rangu.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_File file;
    MPI_Datatype column;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    const int n = size;
    const MPI_Offset file_size = (MPI_Offset)n * n;
    const MPI_Offset offset = (MPI_Offset)(size - 1 - rank) * n;
    char text[n];

    for (int i = 0; i < n; i++)
        text[i] = 'A' + rank % 26;

    MPI_File_open(MCW, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, text, n, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_Barrier(MCW);
    MPI_File_set_size(file, file_size);
    MPI_File_close(&file);

    MPI_Barrier(MCW);
    MPI_File_open(MCW, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read_shared(file, text, n, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    MPI_Type_vector(n, 1, n, MPI_CHAR, &column);
    MPI_Type_commit(&column);
    MPI_File_open(MCW, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_view(file, rank, MPI_CHAR, column, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, text, n, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_Barrier(MCW);
    MPI_File_set_size(file, file_size);
    MPI_File_close(&file);

    MPI_Finalize();
    return 0;
}
