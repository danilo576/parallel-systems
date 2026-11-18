#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Ispit 20.06.2024, zadatak 1.
 * Napisati MPI program koji za niz realnih brojeva A računa
 * M = suma od i = 1 do N izraza (A[i] + As) / C, gde je As srednja
 * vrednost elemenata niza A. Parametar C nalazi se u master procesu,
 * a niz A u datoteci niz.dat. Svi procesi učestvuju u računanju i
 * učitavaju deo niza A. Pretpostaviti da je N deljivo brojem procesa.
 * Redosled učitavanja nije definisan i svaki proces može pročitati bilo
 * koji element niza. Paralelizovati sve moguće operacije. Rezultat
 * upisati u proces koji je učitao najveći element niza A.
 */
int main(int argc, char *argv[])
{
    int rank, process_count;
    enum { MEAN, DIVISOR, PARAMETER_COUNT };
    double parameters[PARAMETER_COUNT];
    MPI_File input;
    MPI_Offset file_size;
    struct {
        double value;
        int rank;
    } local_max, global_max;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    if (rank == 0)
        parameters[DIVISOR] = 2.0;

    MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_ARE_FATAL);
    MPI_File_open(MPI_COMM_WORLD, "niz.dat", MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &input);
    MPI_File_get_size(input, &file_size);

    MPI_Offset block_bytes = (MPI_Offset)process_count * sizeof(double);
    if (file_size <= 0 || file_size % block_bytes != 0 ||
        file_size / block_bytes > INT_MAX) {
        if (rank == 0)
            fprintf(stderr, "Input must contain a positive multiple of the process count in doubles, with at most INT_MAX doubles per process.\n");
        MPI_File_close(&input);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_count = (int)(file_size / block_bytes);
    double *values = malloc((size_t)local_count * sizeof(*values));
    if (values == NULL) {
        fprintf(stderr, "Process %d could not allocate the input buffer.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    MPI_File_set_view(input, 0, MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);
    MPI_File_read_shared(input, values, local_count, MPI_DOUBLE, MPI_STATUS_IGNORE);
    MPI_File_close(&input);

    double local_sum = 0.0;
    local_max.value = values[0];
    local_max.rank = rank;
    for (int i = 0; i < local_count; ++i) {
        local_sum += values[i];
        if (values[i] > local_max.value)
            local_max.value = values[i];
    }

    double total_sum;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM,
               0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_DOUBLE_INT, MPI_MAXLOC,
               0, MPI_COMM_WORLD);
    int result_rank;
    if (rank == 0) {
        parameters[MEAN] = total_sum / (file_size / sizeof(double));
        result_rank = global_max.rank;
    }
    MPI_Bcast(parameters, PARAMETER_COUNT, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&result_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double local_result = 0.0;
    for (int i = 0; i < local_count; ++i)
        local_result += (values[i] + parameters[MEAN]) / parameters[DIVISOR];
    free(values);

    double result;
    MPI_Reduce(&local_result, &result, 1, MPI_DOUBLE, MPI_SUM,
               result_rank, MPI_COMM_WORLD);
    if (rank == result_rank)
        printf("Process %d: M = %.10g\n", rank, result);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
