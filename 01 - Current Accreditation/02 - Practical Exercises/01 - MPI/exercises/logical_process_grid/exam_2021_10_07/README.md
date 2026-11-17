# Zadatak 2 — 07.10.2021.

[Rešenje](mpi_cyclic_rows_matrix_vector.c) množi matricu A i vektor b, uz cikličnu raspodelu vrsta i blokovsku raspodelu kolona. Oznaka `N` u kodu odgovara oznaci `k` iz teksta zadatka. Podrazumevano je `N = 8`; potrebno je `p = q*q` i `N % q == 0`.

![Raspodela matrice i komunikacije](matrix_vector_sketch.png)

Skica je generisana ugrađenim `imagegen` alatom, u stilu skice zadatka od 03.07.2020. [Korišćeni prompt](matrix_vector_sketch_prompt.txt) sačuvan je uz sliku.

Strelice označavaju logički tok grupnih operacija, a ne njihov interni algoritam. Svaki proces ima posebnu boju, istu u matrici i mreži procesa.

## 1. Raspodela podataka

Za `row = rank/q`, `col = rank%q` i `s = N/q` važi:

```c
local_a[i][j] = a[row + i*q][col*s + j];
local_b[j] = b[col*s + j];
```

Za `N=8`, `p=16`, `q=4`, `s=2`:

| Proces | Vrste A | Kolone A | Deo b |
| --- | --- | --- | --- |
| P0 | 0, 4 | 0, 1 | b[0], b[1] |
| P1 | 0, 4 | 2, 3 | b[2], b[3] |
| P4 | 1, 5 | 0, 1 | b[0], b[1] |
| P15 | 3, 7 | 6, 7 | b[6], b[7] |

`MPI_Comm_split` formira komunikatore vrsta (`row_comm`, lokalni rang je `col`) i kolona (`col_comm`, lokalni rang je `row`).

## 2. Slanje matrice jednom grupnom operacijom

```c
MPI_Type_vector(s, s, q*N, MPI_DOUBLE, &block_type);
MPI_Type_create_resized(block_type, 0, s * sizeof(double), &block_resized);
```

Tip bira `s` grupa od po `s` uzastopnih elemenata; početke grupa razdvaja `q*N` elemenata. Za P1 su to `a[0][2]`, `a[0][3]`, `a[4][2]`, `a[4][3]`.

`MPI_Scatter` šalje po jedan `block_resized` svakom procesu. Extent od `s * sizeof(double)` pomera početak sledećeg bloka za `s` elemenata. Početak bloka procesa `rank` zato je na pomeraju

```text
rank*s = (row*q+col)*s = row*N + col*s
```

To je upravo adresa `a[row][col*s]`. Za 16 procesa počeci su `a[0][0]`, `a[0][2]`, `a[0][4]`, `a[0][6]`, zatim `a[1][0]` itd. Izvedeni tip iz svake početne adrese bira odgovarajuće ciklične vrste.

Svaki proces dobija ceo blok odjednom, u gustu lokalnu matricu `s × s`. I master prima svoj blok kroz isti poziv. Nisu potrebni nizovi brojača i pomeraja.


## 3. Raspodela vektora b

U prvoj vrsti mreže `MPI_Scatter` deli b na uzastopne delove dužine `s`. Zatim `MPI_Bcast` po svakoj koloni dostavlja isti deo svim procesima te kolone.

## 4. Množenje i izbor procesa za ispis

Svaki proces računa `local_c[i] = Σ local_a[i][j] * local_b[j]`. To je delimični doprinos elementu `c[row+i*q]`.

Istovremeno nalazi minimum svog bloka. `MPI_Reduce` sa `MPI_MINLOC` nad parom `(vrednost, svetski rang)` bira vlasnika globalnog minimuma, a `MPI_Bcast` prosleđuje taj par svima. Pri jednakim minimumima bira se najmanji rang.

## 5. Sklapanje rezultata

1. Koordinate izabranog procesa su `result_row = global_min.rank/q` i `result_col = global_min.rank%q`.
2. `MPI_Reduce` sa `MPI_SUM` u svakoj vrsti mreže sabira doprinose u proces sa lokalnim rangom `result_col`.
3. Samo ta kolona poziva `MPI_Gather`, sa korenom `result_row`. Prijemni tip `MPI_Type_vector(s, 1, q, ...)`, sa extent-om jednog elementa, raspoređuje deo procesa vrste r na mesta `c[r], c[r+q], ...`. Običan Gather sa uzastopnim prijemom dao bi pogrešan redosled.

Za P15, delovi pristižu od P3, P7, P11 i P15, na indekse `{0,4}`, `{1,5}`, `{2,6}` i `{3,7}`.

## Pokretanje i provera

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_cyclic_rows_matrix_vector.c -o mpi_cyclic_rows_matrix_vector
mpiexec --oversubscribe -np 16 ./mpi_cyclic_rows_matrix_vector
```

Inicijalizacija je `a[i][j] = i*N+j+1`, `b[j] = j+1`, uz `a[N-1][N-1] = -1`. Očekivani izlaz:

```text
Result process: 15
Minimum A: -1
Vector C: 204 492 780 1068 1356 1644 1932 1700
```

Kompajliranje prolazi bez upozorenja. Rezultat je proveren za 4 i 16 procesa i odgovara sekvencijalnom proizvodu; za 4 procesa minimum poseduje P3.

## Lokalni izvori

- `../exam_2020_07_03/mpi_block_matrix_vector_b.c`: komunikatori vrsta i kolona, raspodela grupnim operacijama, MINLOC i izbor korena.
- `../exam_2019_06_12/mpi_block_matrix_vector.c`: lokalni proizvod i sabiranje doprinosa po vrstama mreže.
- `04 - Additional Materials/Paralelni sistemi/elfak-parallel-systems-main/Priprema/MPI/apr-2023-a/Source.c` (relativno od `01 - Current Accreditation`): izvedeni tipovi, Scatter/Bcast i obrazac za MINLOC. Raspodela i lokalni koreni prilagođeni su ovom zadatku.
