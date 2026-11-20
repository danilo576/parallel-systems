#include <math.h>
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define NAME_LENGTH 32
#define EMPLOYEES_PER_PROCESS 2

typedef struct {
    int id;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    float average_salary;
} Employee;

/*
 * 28.10.2025, zadatak b
 *
 * Napisati MPI program koji kreiranjem komunikatora omogućava
 * komunikaciju samo procesima u donjem trouglu kvadratne mreže,
 * bez glavne dijagonale. Svaki proces te grupe treba da dobije
 * jednak broj elemenata niza struktura zaposlenih. Svaka struktura
 * sadrži matični broj, ime, prezime i prosečnu platu.
 */
int main(int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    const int q = (int)sqrt(size);
    if (q * q != size || q < 2) {
        if (rank == 0)
            fprintf(stderr, "Use a square process grid of at least 2 by 2.\n");
        MPI_Finalize();
        return 1;
    }

    int lower_ranks[size];
    int lower_count = 0;
    for (int process = 0; process < size; process++)
        if (process / q > process % q)
            lower_ranks[lower_count++] = process;

    MPI_Group world_group, lower_group;
    MPI_Comm lower_comm;
    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, lower_count, lower_ranks, &lower_group);
    MPI_Comm_create(MCW, lower_group, &lower_comm);

    if (rank / q > rank % q) {
        Employee *employees = NULL;
        Employee local_employees[EMPLOYEES_PER_PROCESS];

        if (rank == lower_ranks[0]) {
            int employee_count = lower_count * EMPLOYEES_PER_PROCESS;
            employees = malloc((size_t)employee_count * sizeof(*employees));
            for (int i = 0; i < employee_count; i++)
                employees[i] = (Employee){
                    1001 + i, "Ana", "Anic", 50000.0f + 1000.0f * i
                };
        }

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

        MPI_Scatter(employees, EMPLOYEES_PER_PROCESS, employee_type,
                    local_employees, EMPLOYEES_PER_PROCESS, employee_type, 0, lower_comm);
        free(employees);

        for (int i = 0; i < EMPLOYEES_PER_PROCESS; i++) {
            Employee employee = local_employees[i];
            printf("P%d: %d %s %s %.2f\n", rank, employee.id, employee.first_name,
                   employee.last_name, employee.average_salary);
        }
    }

    MPI_Finalize();
    return 0;
}
