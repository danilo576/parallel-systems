#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define NAME_LENGTH 100

typedef struct {
    int id;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    float salary;
} Employee;

/*
 * Ispit 22. 6. 2023, zadatak 1.
 * Napisati MPI program koji nalazi zaposlenog sa najmanjom prosečnom platom.
 * Master proces sa tastature unosi identifikator, ime, prezime i prosečnu
 * platu svakog od n zaposlenih. Svaki od p procesa odjednom dobija podatke
 * o n/p zaposlenih i učestvuje u nalaženju minimuma. Master ispisuje
 * identifikator i prosečnu platu izabranog zaposlenog.
 * Koristiti grupne operacije i izvedene tipove podataka.
 */
int main(int argc, char *argv[])
{
    int rank, process_count, employee_count = 0;
    Employee *employees = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &process_count);

    if (rank == 0)
        scanf("%d", &employee_count);

    MPI_Bcast(&employee_count, 1, MPI_INT, 0, MCW);
    if (employee_count <= 0 || employee_count % process_count != 0) {
        if (rank == 0)
            fprintf(stderr, "The employee count must be positive and divisible by the process count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int local_count = employee_count / process_count;
    Employee *local_employees = malloc((size_t)local_count * sizeof *local_employees);

    if (rank == 0) {
        employees = calloc((size_t)employee_count, sizeof *employees);
        for (int i = 0; i < employee_count; i++) {
            Employee *employee = &employees[i];
            scanf("%d %99s %99s %f", &employee->id, employee->first_name,
                  employee->last_name, &employee->salary);
        }
    }

    int lengths[] = {1, NAME_LENGTH, NAME_LENGTH, 1};
    MPI_Aint offsets[] = {offsetof(Employee, id), offsetof(Employee, first_name),
                          offsetof(Employee, last_name), offsetof(Employee, salary)};
    MPI_Datatype fields[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};
    MPI_Datatype employee_fields, employee_type;
    MPI_Type_create_struct(4, lengths, offsets, fields, &employee_fields);
    MPI_Type_create_resized(employee_fields, 0, sizeof(Employee), &employee_type);
    MPI_Type_commit(&employee_type);

    MPI_Scatter(employees, local_count, employee_type,
                local_employees, local_count, employee_type, 0, MCW);

    float local_min_salary = local_employees[0].salary;
    for (int i = 1; i < local_count; i++) {
        if (local_employees[i].salary < local_min_salary)
            local_min_salary = local_employees[i].salary;
    }

    float min_salary;
    MPI_Reduce(&local_min_salary, &min_salary, 1, MPI_FLOAT, MPI_MIN, 0, MCW);

    if (rank == 0) {
        for (int i = 0; i < employee_count; i++) {
            if (employees[i].salary == min_salary) {
                printf("Employee ID: %d, average salary: %.2f\n", employees[i].id, min_salary);
                break;
            }
        }
    }

    free(local_employees);
    free(employees);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
