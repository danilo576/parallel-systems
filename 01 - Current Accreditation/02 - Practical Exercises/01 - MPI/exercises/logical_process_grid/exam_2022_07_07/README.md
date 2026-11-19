# Prosleđivanje niza studenata po dijagonali — 07.07.2022, zadatak 2b

[Rešenje u C-u](mpi_diagonal_student_broadcast.c) · [Zadatak 2a](../../matrix_vector_products/exam_2022_07_07/README.md)

## Postupak

1. Za p procesa proverava se da je p = q². U logičkoj mreži q × q, proces sa svetskim rangom r ima koordinate `r / q` i `r % q`. Glavnu dijagonalu čine procesi kod kojih su te koordinate jednake.
2. Svaki proces sastavlja istu listu dijagonalnih rangova, gde je rang na poziciji i jednak `i*q + i`. `MPI_Comm_group` vraća grupu komunikatora `MPI_COMM_WORLD`, a `MPI_Group_incl` iz nje izdvaja rangove sa liste. Svi procesi pozivaju `MPI_Comm_create` sa tom grupom. Procesi van dijagonale dobijaju `MPI_COMM_NULL`. P0 je prvi u listi, pa ima lokalni rang 0 u komunikatoru DIAG.
3. `MPI_Type_create_struct` opisuje broj indeksa, dva znakovna niza i prosečnu ocenu. `offsetof` daje stvarne pomeraje polja, a extent `sizeof(Student)` obezbeđuje da MPI pravilno pređe sa jednog zapisa na sledeći u nizu.
4. P0 inicijalizuje dva studenta. Članovi DIAG pozivaju jedan `MPI_Bcast` sa brojem zapisa 2 i korenom lokalnog ranga 0. Za mrežu 2 × 2 ceo niz imaju P0 i P3 i oba procesa ispisuju oba zapisa.

## Skica

![Mreža 2 × 2 i prosleđivanje celog niza od P0 do P3](diagonal_students_group_sketch.png)

[Prompt za skicu](diagonal_students_group_sketch_prompt.txt)

## Prevođenje i pokretanje

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_diagonal_student_broadcast.c -lm -o mpi_diagonal_student_broadcast
mpirun -np 4 ./mpi_diagonal_student_broadcast
```

Proverene su mreže sa 1, 4 i 9 procesa. U mreži 3 × 3 oba zapisa ispisuju samo P0, P4 i P8. Pokretanje sa 2 procesa odbija se jer broj procesa nije potpun kvadrat.

## Lokalni izvori

- [Rešenje iz dopunskih materijala](../../../../../04%20-%20Additional%20Materials/Paralelni%20sistemi/elfak-parallel-systems-main/Priprema/MPI/jun2-2022/Zad2.cpp)
- [Primer prosleđivanja niza struktura](../../basic_communication/exam_2019_06_12/mpi_employee_broadcast.c)
