#include <math.h>
#include <mpi.h>
#include <stdio.h>

enum { N = 4 };

/*
 * Zadatak b, 06.07.2026.
 * Formirati komunikator samo za procese u drugoj vrsti kvadratne
 * mreže dimenzija sqrt(p) × sqrt(p), gde je p ukupan broj procesa.
 * Master te grupe šalje svakom članu jednak broj elemenata sporedne
 * dijagonale matrice X dimenzija n × n, direktno iz X, grupnom
 * operacijom. Broj n je deljiv sa sqrt(p). Prikazati identifikatore
 * članova novog komunikatora i maksimum dodeljenih elemenata u
 * svakom od tih procesa.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    MPI_Comm row_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    int grid_size = (int)sqrt(process_count);
    if (grid_size < 2 || grid_size * grid_size != process_count ||
        N % grid_size != 0) {
        if (rank == 0) {
            fprintf(stderr, "Use a square process grid with at least two rows and a side dividing N.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int row = rank / grid_size;
    int column = rank % grid_size;
    MPI_Comm_split(MPI_COMM_WORLD, row == 1 ? 0 : MPI_UNDEFINED,
                   column, &row_comm);

    if (row_comm != MPI_COMM_NULL) {
        int row_rank;
        int count = N / grid_size;
        int local_diagonal[count];
        int x[N][N];
        MPI_Datatype segment, send_type = MPI_INT;

        MPI_Comm_rank(row_comm, &row_rank);
        if (row_rank == 0) {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    x[i][j] = i * N + j + 1;
                }
            }

            MPI_Type_vector(count, 1, N - 1, MPI_INT, &segment);
            MPI_Type_create_resized(segment, 0,
                                    (MPI_Aint)count * (N - 1) * sizeof(int),
                                    &send_type);
            MPI_Type_commit(&send_type);
        }

        MPI_Scatter(&x[0][N - 1], 1, send_type, local_diagonal, count,
                    MPI_INT, 0, row_comm);

        int maximum = local_diagonal[0];
        for (int i = 1; i < count; i++) {
            if (local_diagonal[i] > maximum) {
                maximum = local_diagonal[i];
            }
        }
        printf("World rank %d, row rank %d: local maximum = %d\n",
               rank, row_rank, maximum);
    }

    MPI_Finalize();
    return 0;
}
