# Paralelni sistemi — 12.09.2025, zadatak 1

## 1a — Ciklična raspodela vrsta

[Rešenje u C-u](mpi_cyclic_row_products.c)

1. P0 inicijalizuje matricu A i vektor b. Tekst navodi da je k deljivo sa p, a niz indeksa koji se završava sa i + m − p podrazumeva i da je m deljivo sa p. Program proverava oba uslova.
2. Tip `MPI_Type_vector(m/p, k, p*k, MPI_INT, ...)` opisuje m/p celih vrsta, sa korakom od p vrsta. `MPI_Type_create_resized` postavlja extent na k elemenata, pa `MPI_Scatter` za P_i počinje od vrste i. `MPI_Bcast` šalje ceo b svim procesima. Obe operacije se izvršavaju u MCW sa korenom 0.
3. Svaki proces računa skalarne proizvode svojih vrsta sa b i lokalni proizvod elemenata svake kolone. Za dva procesa P0 računa c[0] = 10 i c[2] = 42, a P1 računa c[1] = 26 i c[3] = 58.
4. `MPI_Gather` u MCW sa korenom 0 koristi prijemni tip sa korakom od p elemenata i extentom od jednog elementa. Tako rezultat procesa P_i odmah odlazi na globalne indekse i, i + p, …, i + m − p.
5. `MPI_Reduce` sa `MPI_PROD` u MCW množi doprinose po kolonama i vraća ih procesu P0. Na primer, u koloni 0 spaja proizvode 1 · 9 i 5 · 13 i dobija 585.

```sh
mpicc -std=c99 -Wall -Wextra mpi_cyclic_row_products.c -o mpi_cyclic_row_products
mpirun -np 2 ./mpi_cyclic_row_products
```

```text
A * b: 10 26 42 58
Column products: 585 1680 3465 6144
```

## 1b — Komunikator i najmanja suma kolone

[Rešenje u C-u](mpi_column_group_min.c)

1. Vrednost `x = 2` zadata je u kodu. Za pozitivan x, `MPI_Group_incl` bira globalne rangove 0, x, 2x, …, manje od p. Svi procesi pozivaju `MPI_Comm_create` u MCW. Članovi dobijaju lokalne rangove redom, a P0 ima lokalni rang 0. Dalje grupne operacije pozivaju samo procesi za koje važi `rank % x == 0`.
2. Pošto zadatak ne određuje vezu između kolona i rangova, proces lokalnog ranga j dobija kolonu j. Ako ima više kolona nego članova, koriste se prve kolone. Matrica ima najmanje četiri kolone. Ako grupa ima više od četiri člana, broj kolona jednak je broju članova. Za devet procesa i x = 2, matrica ima pet kolona, članovi su P0, P2, P4, P6 i P8, a minimum suma je 34.
3. Tip `MPI_Type_vector(m, 1, k, MPI_INT, ...)` opisuje jednu kolonu. Extent od jednog elementa omogućava da `MPI_Scatter` u `column_comm`, sa korenom 0, svaku sledeću kolonu pošalje sledećem članu.
4. Svaki član sabira elemente primljene kolone. Za p = 4 i x = 2, P0 dobija sumu 28, a P2 sumu 32. `MPI_Reduce` sa `MPI_MIN` u `column_comm` vraća 28 procesu P0, koji štampa globalne rangove članova i minimum.

```sh
mpicc -std=c99 -Wall -Wextra mpi_column_group_min.c -o mpi_column_group_min
mpirun -np 4 ./mpi_column_group_min
```

```text
Group ranks in MCW: 0 2
Minimum column sum: 28
```

## Skica

![Ciklična raspodela vrsta i raspodela kolona u novom komunikatoru](distribution_sketch.png)
