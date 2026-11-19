#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

/*
 * Prvi kolokvijum, 23.04.2024, zadatak 1.
 * Napisati MPI program koji množi matricu A dimenzija n x k i matricu B
 * dimenzija k x m. Master šalje svakom procesu celu matricu A i odjednom
 * svih m/p dodeljenih kolona matrice B, gde je m deljivo brojem procesa p.
 * Svi procesi učestvuju u računanju. Matricu C sklapa i prikazuje proces
 * koji nakon raspodele poseduje maksimalni element matrice B.
 * Dati primer matrica A i B i ilustrovati raspodelu podataka i formiranje C.
 * Deo a) Koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int dimensions[3];
    if (rank == 0)
        scanf("%d %d %d", &dimensions[0], &dimensions[1], &dimensions[2]);
    MPI_Bcast(dimensions, 3, MPI_INT, 0, MCW);

    int n = dimensions[0], k = dimensions[1], m = dimensions[2];
    if (n <= 0 || k <= 0 || m <= 0 || m % size != 0)
    {
        if (rank == 0)
            fprintf(stderr, "Expected positive dimensions and m divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int width = m / size;
    int *a = malloc((size_t)n * k * sizeof(*a));
    int *b = NULL;
    int *local_b = malloc((size_t)k * width * sizeof(*local_b));
    int *local_c = malloc((size_t)n * width * sizeof(*local_c));

    if (rank == 0)
    {
        b = malloc((size_t)k * m * sizeof(*b));
        for (int i = 0; i < n * k; i++)
            scanf("%d", &a[i]);
        for (int i = 0; i < k * m; i++)
            scanf("%d", &b[i]);
    }
    MPI_Bcast(a, n * k, MPI_INT, 0, MCW);

    MPI_Datatype b_block, b_strip;
    MPI_Type_vector(k, width, m, MPI_INT, &b_block);
    MPI_Type_create_resized(b_block, 0, (MPI_Aint)width * sizeof(int), &b_strip);
    MPI_Type_commit(&b_strip);
    MPI_Scatter(b, 1, b_strip, local_b, k * width, MPI_INT, 0, MCW);
    free(b);

    int local_max[2] = {local_b[0], rank};
    for (int i = 1; i < k * width; i++)
        if (local_b[i] > local_max[0])
            local_max[0] = local_b[i];

    for (int i = 0; i < n; i++)
        for (int j = 0; j < width; j++)
        {
            int sum = 0;
            for (int t = 0; t < k; t++)
                sum += a[i * k + t] * local_b[t * width + j];
            local_c[i * width + j] = sum;
        }

    int global_max[2];
    MPI_Reduce(local_max, global_max, 1, MPI_2INT, MPI_MAXLOC, 0, MCW);
    int result_rank;
    if (rank == 0)
        result_rank = global_max[1];
    MPI_Bcast(&result_rank, 1, MPI_INT, 0, MCW);

    MPI_Datatype c_block, c_strip;
    MPI_Type_vector(n, width, m, MPI_INT, &c_block);
    MPI_Type_create_resized(c_block, 0, (MPI_Aint)width * sizeof(int), &c_strip);
    MPI_Type_commit(&c_strip);

    int *c = NULL;
    if (rank == result_rank)
        c = malloc((size_t)n * m * sizeof(*c));

    MPI_Gather(local_c, n * width, MPI_INT, c, 1, c_strip, result_rank, MCW);
    if (rank == result_rank)
    {
        printf("Matrix C (rank %d):\n", rank);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
                printf("%s%d", j == 0 ? "" : " ", c[i * m + j]);
            printf("\n");
        }
    }

    free(c);
    free(local_c);
    free(local_b);
    free(a);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
