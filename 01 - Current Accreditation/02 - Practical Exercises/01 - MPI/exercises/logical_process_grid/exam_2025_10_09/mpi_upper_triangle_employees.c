#include <math.h>
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define NAME_LENGTH 32

typedef struct {
    int id;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    float average_salary;
} Employee;

/*
 * 09.10.2025, zadatak 7b.
 * Kreiranjem komunikatora omogućiti komunikaciju samo procesima
 * u gornjem trouglu kvadratne mreže procesa. Svaki proces ove
 * grupe treba da dobije po jednu strukturu sa podacima o zaposlenom:
 * matični broj, ime, prezime i prosečna plata.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    int q = (int)sqrt(size);
    if (q * q != size) {
        if (rank == 0)
            fprintf(stderr, "Use a square process grid.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int upper_ranks[size];
    int upper_count = 0;
    for (int process = 0; process < size; process++) {
        int row = process / q;
        int col = process % q;

        if (row <= col)
            upper_ranks[upper_count++] = process;
    }

    MPI_Group world_group, upper_group;
    MPI_Comm upper_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, upper_count, upper_ranks, &upper_group);
    MPI_Comm_create(MCW, upper_group, &upper_comm);

    if (rank / q <= rank % q) {
        int lengths[] = {1, NAME_LENGTH, NAME_LENGTH, 1};
        MPI_Aint offsets[] = {
            offsetof(Employee, id),
            offsetof(Employee, first_name),
            offsetof(Employee, last_name),
            offsetof(Employee, average_salary)
        };
        MPI_Datatype fields[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};
        MPI_Datatype employee_fields, employee_type;

        MPI_Type_create_struct(4, lengths, offsets, fields, &employee_fields);
        MPI_Type_create_resized(employee_fields, 0, sizeof(Employee), &employee_type);
        MPI_Type_commit(&employee_type);

        Employee *employees = NULL;
        if (rank == 0) {
            employees = malloc(upper_count * sizeof(*employees));
            for (int i = 0; i < upper_count; i++)
                employees[i] = (Employee){
                    1001 + i, "Ana", "Anic", 50000.0f + 1000.0f * i
                };
        }

        Employee local_employee;
        MPI_Scatter(employees, 1, employee_type, &local_employee, 1,
                    employee_type, 0, upper_comm);

        printf("P%d: %d %s %s %.2f\n", rank, local_employee.id,
               local_employee.first_name, local_employee.last_name,
               local_employee.average_salary);
        free(employees);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
