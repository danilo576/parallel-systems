#include <math.h>
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define ROOT 0
#define NAME_LENGTH 50
#define STUDENT_COUNT 2
#define MCW MPI_COMM_WORLD

typedef struct {
    int index;
    char first_name[NAME_LENGTH];
    char last_name[NAME_LENGTH];
    float average_grade;
} Student;

/*
 * 07.07.2022, zadatak 2b
 *
 * Napisati MPI program koji ceo niz studenata iz master procesa
 * prosleđuje odjednom svim procesima u komunikatoru DIAG. DIAG čine
 * procesi na glavnoj dijagonali kvadratne mreže procesa. Za svakog
 * studenta čuvaju se broj indeksa (int), ime i prezime (string)
 * i prosečna ocena (float). Koristiti grupne operacije.
 */
int main(int argc, char *argv[])
{
    int rank, size;
    Student students[STUDENT_COUNT];
    MPI_Comm diag_comm;
    MPI_Group world_group, diag_group;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    const int side = (int)sqrt(size);
    if (side * side != size) {
        if (rank == ROOT)
            fprintf(stderr, "The number of processes must be a perfect square.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int row = rank / side;
    int col = rank % side;
    int diag_ranks[side];

    for (int i = 0; i < side; i++)
        for (int j = 0; j < side; j++)  
            if (i == j)
                diag_ranks[i] = i * side + j;

    MPI_Comm_group(MCW, &world_group);
    MPI_Group_incl(world_group, side, diag_ranks, &diag_group);
    MPI_Comm_create(MCW, diag_group, &diag_comm);

    if (row == col) {
        if (rank == ROOT) {
            students[0] = (Student){17782, "Petar", "Petrovic", 6.0f};
            students[1] = (Student){17783, "Mika", "Mikic", 9.0f};
        }

        int lengths[] = {1, NAME_LENGTH, NAME_LENGTH, 1};
        MPI_Aint offsets[] = {
            offsetof(Student, index),
            offsetof(Student, first_name),
            offsetof(Student, last_name),
            offsetof(Student, average_grade)
        };
        MPI_Datatype types[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};
        MPI_Datatype student_fields, student_type;

        MPI_Type_create_struct(4, lengths, offsets, types, &student_fields);
        MPI_Type_create_resized(student_fields, 0, sizeof(Student), &student_type);
        MPI_Type_commit(&student_type);

        MPI_Bcast(students, STUDENT_COUNT, student_type, ROOT, diag_comm);

        for (int i = 0; i < STUDENT_COUNT; i++)
            printf("P%d: %d %s %s %.2f\n", rank, students[i].index,
                   students[i].first_name, students[i].last_name,
                   students[i].average_grade);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
