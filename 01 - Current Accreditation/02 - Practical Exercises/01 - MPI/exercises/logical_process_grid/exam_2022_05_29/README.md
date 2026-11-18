# 29.05.2022. — 2.b) Dijagonalni komunikator

1. Za `p = q*q` procesa, globalni rang `rank` određuje koordinate `row = rank / q` i `col = rank % q`. Na glavnoj dijagonali važi `row == col`, pa su njeni globalni rangovi `i*q+i` za `i = 0, …, q-1`.
2. Svi procesi pozivaju `MPI_Comm_group` da dobiju grupu komunikatora `MPI_COMM_WORLD`. Zatim `MPI_Group_incl` iz te grupe izdvaja rangove glavne dijagonale u rastućem redosledu.
3. Svi procesi pozivaju `MPI_Comm_create` sa izdvojenom grupom. Procesi van dijagonale dobijaju `MPI_COMM_NULL`, a P0 u novom komunikatoru dobija lokalni rang 0.
4. P0 postavlja poruku. Svi članovi pozivaju `MPI_Bcast` u `diag_comm` sa lokalnim korenom 0 i prikazuju poruku.

```sh
mpicc -std=c99 -Wall -Wextra mpi_diagonal_communicator.c -lm -o mpi_diagonal_communicator
mpirun -np 4 ./mpi_diagonal_communicator
```

Za mrežu 2 × 2 dijagonalu čine P0 i P3. Redosled ispisa može da se razlikuje.

```text
Process 0 (diagonal rank 0): Hello from the diagonal root!
Process 3 (diagonal rank 1): Hello from the diagonal root!
```

Ljubičasta oznaka u mreži prati glavnu dijagonalu. Plava strelica prikazuje tok poruke od P0 ka P3.

![Glavna dijagonala mreže i slanje poruke unutar dijagonalnog komunikatora](diagonal_sketch.png)

[Zadatak 2.a — Množenje matrica sa cikličnom raspodelom vrsta](../../matrix_matrix_products/exam_2022_05_29/README.md)
