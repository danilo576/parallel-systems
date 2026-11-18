#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    char first_name[32];
    char last_name[32];
    float average_salary;
} Employee;

/*
 * 12.06.2019, zadatak 4a
 *
 * Napisati MPI program kojim se podaci o nizu zaposlenih koji se unose
 * sa tastature u master procesu prosleđuju odjednom svim procesima u
 * komunikatoru MPI_COMM_WORLD. Za svakog zaposlenog pamte se matični
 * broj (int), ime (string), prezime (string) i prosečna plata (float).
 * Zadatak rešiti korišćenjem grupnih operacija i izvedenih tipova
 * podataka.
 */
int main(int argc, char **argv)
{
    int rank;
    int count = 0;
    const int root = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == root) 
        scanf("%d", &count);

    MPI_Bcast(&count, 1, MPI_INT, root, MPI_COMM_WORLD);

    Employee *employees = calloc(count, sizeof(Employee));

    if (rank == root) {
        for (int i = 0; i < count; i++) {
            scanf("%d %31s %31s %f", &employees[i].id,
                  employees[i].first_name, employees[i].last_name,
                  &employees[i].average_salary);
        }
        putchar('\n');
    }

    int block_lengths[] = {1, 32, 32, 1};
    MPI_Aint base_address;
    MPI_Aint displacements[4];

    MPI_Get_address(&employees[0], &base_address);
    MPI_Get_address(&employees[0].id, &displacements[0]);
    MPI_Get_address(&employees[0].first_name, &displacements[1]);
    MPI_Get_address(&employees[0].last_name, &displacements[2]);
    MPI_Get_address(&employees[0].average_salary, &displacements[3]);

    for (int i = 0; i < 4; i++) {
        displacements[i] = MPI_Aint_diff(displacements[i], base_address);
    }

    MPI_Datatype field_types[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};
    MPI_Datatype employee_fields, employee_type;

    MPI_Type_create_struct(4, block_lengths, displacements, field_types,
                           &employee_fields);
    MPI_Type_create_resized(employee_fields, 0, sizeof(Employee), &employee_type);
    MPI_Type_commit(&employee_type);

    MPI_Bcast(employees, count, employee_type, root, MPI_COMM_WORLD);

    for (int i = 0; i < count; i++) {
        printf("Rank %d: %d %s %s %.2f\n", rank, employees[i].id,
               employees[i].first_name, employees[i].last_name,
               employees[i].average_salary);
    }

    free(employees);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
