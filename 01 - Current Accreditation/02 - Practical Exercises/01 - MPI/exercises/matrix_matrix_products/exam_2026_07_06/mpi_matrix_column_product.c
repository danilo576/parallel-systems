#include <mpi.h>
#include <stdio.h>

enum { M = 2, K = 2, N = 4 };

/*
 * Zadatak a, 06.07.2026.
 * Napisati MPI program za množenje matrica A dimenzija m × k i B
 * dimenzija k × n. Master šalje svim procesima celu matricu A i po
 * n/p kolona matrice B, gde je p broj procesa i n je deljivo sa p.
 * Sve elemente dodeljenih kolona poslati odjednom, direktno iz B.
 * Svi procesi učestvuju u računanju matrice C. Koristiti grupne
 * operacije, osim za slanje kolona B, za koje se koriste operacije
 * od tačke do tačke.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    int a[M][K];
    int b[K][N];
    int c[M][N];
    MPI_Datatype b_strip, c_strip, c_receive;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    if (N % process_count != 0) {
        if (rank == 0) {
            fprintf(stderr, "The number of columns must be divisible by the process count.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int width = N / process_count;
    int local_b[K][width];
    int local_c[M][width];

    if (rank == 0) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < K; j++) {
                a[i][j] = i * K + j + 1;
            }
        }
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < N; j++) {
                b[i][j] = i * N + j + 1;
            }
        }
    }

    MPI_Bcast(a, M * K, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        MPI_Type_vector(K, width, N, MPI_INT, &b_strip);
        MPI_Type_commit(&b_strip);
        for (int dest = 1; dest < process_count; dest++) {
            MPI_Send(&b[0][dest * width], 1, b_strip, dest, 0, MPI_COMM_WORLD);
        }
        MPI_Sendrecv(b, 1, b_strip, 0, 0, local_b, K * width, MPI_INT,
                     0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(local_b, K * width, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < width; j++) {
            local_c[i][j] = 0;
            for (int t = 0; t < K; t++) {
                local_c[i][j] += a[i][t] * local_b[t][j];
            }
        }
    }

    MPI_Type_vector(M, width, N, MPI_INT, &c_strip);
    MPI_Type_create_resized(c_strip, 0, (MPI_Aint)width * sizeof(int),
                            &c_receive);
    MPI_Type_commit(&c_receive);

    MPI_Gather(local_c, M * width, MPI_INT, c, 1, c_receive, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        puts("Matrix C:");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d%c", c[i][j], j == N - 1 ? '\n' : ' ');
            }
        }
    }

    MPI_Finalize();
    return 0;
}
