# Ciklično množenje matrica — 14.11.2025, zadatak 6a

[Rešenje u C-u](mpi_cyclic_matrix_product.c) · [Zadatak 6b](../../logical_process_grid/exam_2025_11_14/README.md)

![Ciklična raspodela kolona A i vrsta B i sabiranje parcijalnih proizvoda](cyclic_product_sketch.png)

## Postupak

1. P0 inicijalizuje matrice A dimenzija n × k i B dimenzija k × m. Ako je k deljivo sa p, svaki proces dobija `s = k/p` kolona A i isto toliko vrsta B.
2. Za A koristimo `MPI_Type_vector(n*s, 1, p, MPI_INT, ...)`. Pošto je k deljivo sa p, korak p prelazi iz poslednje izabrane kolone jedne vrste u prvu izabranu kolonu sledeće vrste. Extent od jednog elementa postavlja početak bloka procesa Pi na `A[0][i]`.
3. Za B koristimo `MPI_Type_vector(s, m, p*m, MPI_INT, ...)`. Svaki blok vektorskog tipa opisuje celu vrstu, a extent od m elemenata postavlja početak bloka procesa Pi na `B[i][0]`. Po jedan poziv `MPI_Scatter` za svaku matricu čita direktno iz nje i smešta elemente u kontinualne lokalne matrice. Oba poziva koriste `MPI_COMM_WORLD` i koren 0.
4. Proces Pi računa parcijalnu matricu dimenzija n × m. Za element na poziciji (r, c) sabira proizvode `A[r][i+t*p] * B[i+t*p][c]` za `t = 0, …, s−1`.
5. `MPI_Reduce` sa operacijom `MPI_SUM` sabira parcijalne matrice po elementima u P0. Za p = 2, P0 koristi indekse 0 i 2, a P1 indekse 1 i 3. Na primer, prvi element rezultata je 16 + 34 = 50.

## Pokretanje

```sh
mpicc -std=c99 -Wall -Wextra mpi_cyclic_matrix_product.c -o mpi_cyclic_matrix_product
mpirun -np 2 ./mpi_cyclic_matrix_product
```

```text
Matrix C:
50 60
114 140
```
