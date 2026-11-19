#include <mpi.h>

#define MCW MPI_COMM_WORLD

/*
 * Ispit 23. 6. 2022, zadatak 3.
 * Napisati MPI program koji vrši paralelni upis i čitanje binarne datoteke.
 * 1. Svaki proces pomoću pojedinačnog pokazivača upisuje tekst proizvoljnog
 *    sadržaja dužine N u prva.dat. Blokovi se nižu po rastućem rangu.
 * 2. Ponovo otvoriti datoteku. Svaki proces čita N podataka tako da se
 *    unapred ne zna koji će proces pročitati koji deo datoteke.
 *    Obratiti pažnju na konzistentnost pri čitanju.
 * 3. Pročitane podatke upisati u druga.dat kao matricu N × 2N.
 *    Svaki proces upisuje dve susedne kolone, redom po rangu kao na slici
 *    za N = 4. Posebno obratiti pažnju na efikasnost paralelnog upisa.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_File file;
    MPI_Datatype column_pair;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    const int text_length = process_count;
    MPI_Offset offset = (MPI_Offset)rank * text_length;
    MPI_Offset input_size = (MPI_Offset)process_count * text_length * sizeof(char);
    char text[text_length];
    char pairs[2 * text_length];

    for (int i = 0; i < text_length; i++)
        text[i] = 'A' + (offset + i) % 26;

    MPI_File_open(MCW, "prva.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_seek(file, offset, MPI_SEEK_SET);
    MPI_File_write_all(file, text, text_length, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_Barrier(MCW);
    MPI_File_set_size(file, input_size);
    MPI_File_close(&file);

    MPI_Barrier(MCW);
    MPI_File_open(MCW, "prva.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read_shared(file, text, text_length, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    for (int i = 0; i < text_length; i++)
    {
        pairs[2 * i] = text[i];
        pairs[2 * i + 1] = text[i];
    }

    MPI_Type_vector(text_length, 2, 2 * process_count, MPI_CHAR, &column_pair);
    MPI_Type_commit(&column_pair);

    MPI_File_open(MCW, "druga.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 2 * (MPI_Offset)rank, MPI_CHAR, column_pair,
                      "native", MPI_INFO_NULL);
    MPI_File_write_all(file, pairs, 2 * text_length, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_Barrier(MCW);
    MPI_File_set_size(file, 2 * input_size);
    MPI_File_close(&file);

    MPI_Finalize();
    return 0;
}
