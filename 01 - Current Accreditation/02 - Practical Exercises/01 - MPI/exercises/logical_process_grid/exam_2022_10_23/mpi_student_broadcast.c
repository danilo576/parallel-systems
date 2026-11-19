#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
#define NAME_SIZE 32

typedef struct {
    int index;
    char first_name[NAME_SIZE];
    char last_name[NAME_SIZE];
    float average_grade;
    int study_year;
} Student;

/*
 * 2b) Master učitava niz studenata sa tastature i odjednom ga
 * prosleđuje svim procesima u MPI_COMM_WORLD. Za svakog studenta pamte
 * se broj indeksa, ime, prezime, prosečna ocena i godina studija.
 * Koristiti grupne operacije i izvedene tipove podataka.
 */
int main(int argc, char **argv)
{
    int rank, count;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);

    if (rank == 0)
        scanf("%d", &count);
    MPI_Bcast(&count, 1, MPI_INT, 0, MCW);

    Student *students = malloc(count * sizeof *students);
    if (rank == 0)
        for (int i = 0; i < count; i++)
            scanf("%d %31s %31s %f %d", &students[i].index,
                  students[i].first_name, students[i].last_name,
                  &students[i].average_grade, &students[i].study_year);

    int lengths[] = {1, NAME_SIZE, NAME_SIZE, 1, 1};
    MPI_Aint offsets[] = {
        offsetof(Student, index),
        offsetof(Student, first_name),
        offsetof(Student, last_name),
        offsetof(Student, average_grade),
        offsetof(Student, study_year)
    };
    MPI_Datatype types[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT, MPI_INT};
    MPI_Datatype fields, student_type;
    MPI_Type_create_struct(5, lengths, offsets, types, &fields);
    MPI_Type_create_resized(fields, 0, sizeof(Student), &student_type);
    MPI_Type_commit(&student_type);

    MPI_Bcast(students, count, student_type, 0, MCW);

    for (int i = 0; i < count; i++)
        printf("Process %d: %d %s %s %.2f %d\n", rank, students[i].index,
               students[i].first_name, students[i].last_name,
               students[i].average_grade, students[i].study_year);

    free(students);
    MPI_Finalize();
    return 0;
}
