# Donji trougao mreže procesa — 14.11.2025, zadatak 6b

[Rešenje u C-u](mpi_lower_triangle_diagonal.c) · [Zadatak 6a](../../matrix_matrix_products/exam_2025_11_14/README.md)

![Donji trougao mreže i slanje glavne dijagonale iz P0](lower_triangle_group_sketch.png)

## Postupak

1. Za p = q² procesa, polju `(i, j)` odgovara globalni rang `i*q + j`. Dve petlje upisuju u `lower_ranks` sve rangove za koje važi `i >= j`. Za mrežu 2 × 2 to je lista `[0, 2, 3]`.
2. `MPI_Comm_group` izdvaja grupu iz `MPI_COMM_WORLD`, a `MPI_Group_incl` od rangova iz liste pravi donju grupu. Svi procesi pozivaju `MPI_Comm_create`. Članovi grupe dobijaju `lower_comm`, a ostali dobijaju `MPI_COMM_NULL`. P0 je prvi u listi, pa ima lokalni rang 0 i inicijalizuje matricu A.
3. Svaki član koristi matricu `a[q][q]` i isti izvedeni tip `MPI_Type_vector(q, 1, q+1, MPI_INT, ...)`. Tip opisuje q elemenata na razmaku q + 1. Za q = 2 bira memorijske pozicije 0 i 3.
4. Svi članovi pozivaju `MPI_Bcast(a, 1, diagonal_type, 0, lower_comm)`. P0 šalje direktno iz svoje matrice, a MPI upisuje primljene vrednosti na dijagonalu matrice svakog primaoca. Ispisuju se samo elementi `a[i][i]`.

## Pokretanje

```sh
mpicc -std=c99 -Wall -Wextra mpi_lower_triangle_diagonal.c -o mpi_lower_triangle_diagonal -lm
mpirun -np 4 ./mpi_lower_triangle_diagonal
```

Mogući redosled ispisa:

```text
P0 diagonal: 1 4
P2 diagonal: 1 4
P3 diagonal: 1 4
```
