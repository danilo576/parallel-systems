# Donji trougao mreže procesa — 14.11.2025, zadatak 6b

[Rešenje u C-u](mpi_lower_triangle_diagonal.c) · [Zadatak 6a](../../matrix_matrix_products/exam_2025_11_14/README.md)

![Donji trougao mreže i slanje glavne dijagonale iz P0](lower_triangle_sketch.png)

## Postupak

1. Za p = q² procesa, vrsta procesa je `rank/q`, a kolona `rank%q`. Procesi za koje važi `row >= col` dobijaju boju 0. Ostali prosleđuju `MPI_UNDEFINED`.
2. Svi procesi pozivaju `MPI_Comm_split`. Procesi sa bojom 0 dobijaju `lower_comm`, a ostali dobijaju `MPI_COMM_NULL`. Ključ `rank` uređuje članove po globalnom rangu. U mreži 2 × 2 članovi su P0, P2 i P3, pa P0 ima lokalni rang 0 i inicijalizuje matricu A.
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
