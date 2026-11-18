# 29.05.2022. — 2.a) Množenje matrica sa cikličnom raspodelom vrsta

Matrice A i B inicijalizuje P0. Proces Pi dobija vrste A sa globalnim indeksima `i + t*p`, za `t = 0, …, k/p - 1`. Svaki proces dobija i celu matricu B.

1. `MPI_Type_vector(k/p, n, p*n, MPI_INT, …)` opisuje sve vrste A koje pripadaju jednom procesu. Veličina bloka je `n` elemenata, a početke susednih blokova deli `p*n` elemenata.
2. `MPI_Type_create_resized` postavlja extent na veličinu jedne vrste A. Zato `MPI_Scatter` za Pi kreće od vrste `i` i šalje sve njegove vrste direktno iz A. Proces prima `(k/p)*n` uzastopnih elemenata u lokalnu matricu.
3. `MPI_Bcast` šalje celu B sa P0. Svaki proces za svoje vrste računa `C[i][j] = Σ A[i][t] * B[t][j]`, gde `t` prolazi od 0 do `n-1`.
4. `MPI_Gather` na P0 koristi prijemni tip sa `k/p` blokova dužine `m`, korakom `p*m` i extentom jedne vrste C. Tako vrste pri prijemu odmah zauzimaju globalne indekse `i, i+p, …`. Sve tri kolektivne operacije koriste `MPI_COMM_WORLD` i koren 0.

Za dva procesa P0 računa vrste 0 i 2, a P1 vrste 1 i 3. Na primer, P0 računa `C[2][1] = 5*2 + 6*1 = 16`.

```sh
mpicc -std=c99 -Wall -Wextra mpi_cyclic_row_matrix_product.c -o mpi_cyclic_row_matrix_product
mpirun -np 2 ./mpi_cyclic_row_matrix_product
```

```text
Matrix C:
1 4
3 10
5 16
7 22
```

Rezultujuća matrica C na desnoj strani nalazi se kod P0. Boje označavaju koji je proces izračunao pojedine vrste.

![Raspodela vrsta, lokalno množenje i sklapanje matrice C](matrix_product_sketch.png)

[Zadatak 2.b — Dijagonalni komunikator](../../logical_process_grid/exam_2022_05_29/README.md)
