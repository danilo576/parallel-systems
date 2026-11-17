# Množenje matrica — 21.10.2025.

Rešenje: [mpi_row_column_matrix_product.c](mpi_row_column_matrix_product.c).

## Zadatak

Napisati MPI program za množenje A(n×n) i B(n×n), čime se dobija C(n×n). Master inicijalizuje matrice. Procesi su uređeni u mrežu q×q, gde je p=q² i q=n. Svaki proces u i-toj vrsti mreže dobija celu i-tu vrstu A, a svaki proces u j-toj koloni dobija celu j-tu kolonu B. Master šalje deo svake matrice svakom procesu odjednom, direktno iz matrice, point-to-point operacijama. Svaki proces računa svoj deo rezultata. Za ostatak komunikacije koristiti grupne operacije i prikazati rezultat u masteru. Kreiranje novih komunikatora nije dozvoljeno.

## Postupak

1. Koristimo samo `MPI_COMM_WORLD`. Mreža je logička: proces r ima koordinate `i = r / N`, `j = r % N`. Uslov je tačno `N*N` procesa; `N` predstavlja n.
2. Vrsta A je kontinualna, pa master šalje `N` vrednosti tipa `MPI_DOUBLE` iz `a[i]` jednim pozivom.
3. Kolona B nije kontinualna. `MPI_Type_vector(N, 1, N, MPI_DOUBLE, &column_type)` bira N elemenata, po jedan iz svake vrste, uz korak N. Poziv `MPI_Send(&b[0][j], 1, column_type, ...)` šalje celu kolonu direktno iz B, bez pomoćnog pakovanja. Primalac prima N običnih elemenata u kontinualni `local_b`. Potpisi tipova se poklapaju: N elemenata `MPI_DOUBLE`.
4. P0 kopira sopstvenu vrstu i kolonu lokalno. Svakom drugom procesu šalje dve poruke: jednu za A i jednu za B. Posebne oznake poruka su 0 i 1.
5. Svaki proces računa **ceo** element `C[i][j] = Σ local_a[k] * local_b[k]`, za k od 0 do N−1. Ne dobijamo parcijalne sume, pa redukcija nije potrebna.
6. `MPI_Gather` prikuplja po jedan skalar u P0. Redosled rangova je `0,1,...,N*N-1`, a rang `i*N+j` odgovara upravo mestu `c[i][j]` u memoriji. Zato se rezultat direktno sklapa u C, bez preuređivanja.

Nema kreiranja komunikatora niti dodatne sinhronizacije. Za P0 nije potrebno slanje samom sebi.

## Primer

Za n=2:

```text
A = 1 2       B = 1 2
    3 4           3 4
```

| Proces | Koordinate | Vrsta A | Kolona B | Rezultat |
|---|---|---|---|---|
| P0 | (0,0) | 1,2 | 1,3 | C[0][0]=7 |
| P1 | (0,1) | 1,2 | 2,4 | C[0][1]=10 |
| P2 | (1,0) | 3,4 | 1,3 | C[1][0]=15 |
| P3 | (1,1) | 3,4 | 2,4 | C[1][1]=22 |

Na primer, P2 računa `3*1 + 4*3 = 15`. Rezultat u P0 je:

```text
7  10
15 22
```

## Skica

![Raspodela podataka i komunikacija](matrix_product_sketch.png)

Podeljene boje u A i B označavaju kopije celog elementa kod više procesa. Svaki proces ima svoju boju; ta boja prati i njegov element u C. Strelice prikazuju logički tok podataka, a ne internu implementaciju kolektivne operacije.

Prompt: [matrix_product_sketch_prompt.txt](matrix_product_sketch_prompt.txt). Skica je generisana ugrađenim alatom imagegen.

## Prevođenje i pokretanje

Iz ovog foldera:

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_row_column_matrix_product.c -o /tmp/mpi_row_column_matrix_product
mpirun --oversubscribe -np 4 /tmp/mpi_row_column_matrix_product
```

Drugi primer dimenzija:

```sh
mpicc -std=c99 -Wall -Wextra -pedantic -DN=3 mpi_row_column_matrix_product.c -o /tmp/mpi_row_column_matrix_product_3
mpirun --oversubscribe -np 9 /tmp/mpi_row_column_matrix_product_3
```

## Provera

Prevođenje sa `-std=c99 -Wall -Wextra -pedantic` prolazi bez upozorenja. MPI rezultati za `N = 1, 2, 3` poklapaju se sa serijskim proizvodom. Provereno je i odbijanje pokretanja sa pogrešnim brojem procesa.

## Lokalni izvori

- `../exam_2022_06_23/mpi_block_matrix_product.c`: množenje matrica i direktno slanje izvedenim tipom. Ovde je raspodela drugačija i nisu dozvoljeni novi komunikatori.
- `../exam_2021_10_07/mpi_cyclic_rows_matrix_vector.c`: stil rešenja i izvedeni tip `MPI_Type_vector`.
- Provereni su i dopunski materijali u `01 - Current Accreditation/04 - Additional Materials/Paralelni sistemi`; uslovi konkretnog zadatka sa slike određuju ovo rešenje.
