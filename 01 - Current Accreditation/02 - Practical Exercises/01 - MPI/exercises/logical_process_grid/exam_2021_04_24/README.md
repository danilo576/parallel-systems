# Zadatak 3 — 24.04.2021.

[Rešenje](mpi_cyclic_matrix_vector_max.c) množi matricu A i vektor b. Tekst zadatka sa priložene slike nalazi se iznad funkcije `main`. Oznaka `m` iz zadatka u kodu je `N = 8`. Potrebno je `p = q*q` i `N` deljivo sa `q`.

Raspodela je ista kao u [exam_2023_10_02](../exam_2023_10_02/README.md): i vrste i kolone su ciklične. Razlika je u slanju A jednim `MPI_Send` pozivom po primaocu i prikazu c u procesu koji poseduje maksimum A, umesto paralelnog upisa u fajl.

![Raspodela podataka, računanje i komunikacije](matrix_vector_sketch.png)

## 1. Koordinate i raspodela

```c
row = rank / q;
col = rank % q;
rows = cols = N / q;
local_a[i][j] = a[row + i*q][col + j*q];
local_b[j] = b[col + j*q];
```

Za `N = 8`, `p = 16`, lokalni blok je `2 × 2`:

| Proces | Globalne vrste | Globalne kolone | Deo b |
| --- | --- | --- | --- |
| P0 | 0, 4 | 0, 4 | b[0], b[4] |
| P1 | 0, 4 | 1, 5 | b[1], b[5] |
| P4 | 1, 5 | 0, 4 | b[0], b[4] |
| P15 | 3, 7 | 3, 7 | b[3], b[7] |

`MPI_Comm_split(MCW, row, col, &row_comm)` formira vrste mreže; lokalni rang je `col`. Poziv sa `color = col`, `key = row` formira kolone; lokalni rang u `col_comm` je `row`.

## 2. Jedan izvedeni tip za ceo blok A

```c
MPI_Type_vector(cols, 1, q, MPI_DOUBLE, &column_type);
MPI_Type_create_resized(column_type, 0, N * sizeof(double), &column_resized);
MPI_Type_vector(rows, 1, q, column_resized, &block_type);
MPI_Type_commit(&block_type);
```

Prvi tip bira ciklične kolone jedne globalne vrste. Zatim mu postavljamo extent na dužinu jedne cele vrste, `N*sizeof(double)`. Spoljašnji `MPI_Type_vector` ponavlja taj izbor `rows` puta, sa korakom `q` takvih extent-a, odnosno `q*N` elemenata. Time bira globalne vrste `row, row+q, ...`. Dovoljno je potvrditi završni tip koji se koristi u komunikaciji.

Master za svaki `dest != 0` šalje:

```c
MPI_Send(&a[dest / q][dest % q], 1, block_type, dest, 0, MCW);
```

Primalac dobija `rows*cols` uzastopnih `MPI_DOUBLE` vrednosti. Master svoj blok direktno kopira. Na primer, P1 dobija `a[0][1], a[0][5], a[4][1], a[4][5]`.

Jedan običan `MPI_Type_vector(rows*cols, 1, q, ...)` nad originalnom matricom ovde ne bi bio dovoljan: posle završetka prve lokalne vrste mora se preskočiti do sledeće globalne vrste sa indeksom većim za `q`. U rešenju iz 2023. jednostavniji tip radi nad prethodno zapakovanim cikličnim vrstama.

## 3. Raspodela b: Scatter pa Bcast

`vector_type` bira `cols` elemenata sa korakom `q`. `MPI_Type_create_resized` postavlja extent na `sizeof(double)`, pa početak svakog narednog dela pomera za jedan element.

1. Samo prva vrsta mreže poziva `MPI_Scatter` u `row_comm`, sa korenom 0. P0, P1, P2 i P3 dobijaju redom `[1,5]`, `[2,6]`, `[3,7]`, `[4,8]`.
2. Svaka kolona poziva `MPI_Bcast` u svom `col_comm`, sa korenom 0. Deo b tako stiže svim procesima koji poseduju iste globalne kolone A.

## 4. Lokalno računanje i izbor odredišta

Svaki proces računa parcijalne sume za svoje globalne vrste:

```c
local_c[i] += local_a[i][j] * local_b[j];
```

Za inicijalizaciju `a[i][j] = i*N+j+1`, `b[j] = j+1`, P1 ima:

```text
local_a = [ 2  6 ]    local_b = [2, 6]
          [34 38 ]    local_c = [40, 296]
```

To su doprinosi za `c[0]` i `c[4]`, a ne gotove vrednosti. Prva vrsta procesne mreže sabira:

```text
P0 [26,218] + P1 [40,296] + P2 [58,378] + P3 [80,464]
= [204,1356] = [c[0],c[4]]
```

Istovremeno svaki proces pronalazi lokalni maksimum. `MPI_Reduce` sa `MPI_DOUBLE_INT` i `MPI_MAXLOC` vraća vrednost i rang vlasnika maksimuma u P0, a `MPI_Bcast` deli taj par svim procesima. Pri jednakim maksimumima bira se najmanji rang vlasnika. Iz njega se određuju `result_row` i `result_col`.

## 5. Sklapanje u procesu sa maksimumom

1. `MPI_Reduce` sa `MPI_SUM` u svakom `row_comm` sabira parcijalne sume, uz lokalni koren `result_col`.
2. Samo kolona `result_col` poziva `MPI_Gather` u svom `col_comm`, uz lokalni koren `result_row`.

Kod koristi isti `vector_resized` za prijem pri okupljanju:

```c
MPI_Gather(row_c, rows, MPI_DOUBLE, c, 1, vector_resized,
           result_row, col_comm);
```

Prijem jednog tipa smešta par sa lokalnog ranga `r` na indekse `r, r+q, ...`. Extent od jednog elementa pomera početak za sledeći rang. Tako se vektor c odmah sklapa u globalnom redosledu, bez dodatnog preuređivanja. Izabrana mesta se ne preklapaju.

Za 16 procesa maksimum je 64 u `a[7][7]`, kod P15. Zato je koren redukcije po vrstama 3, a koren okupljanja u koloni 3 takođe 3:

| Proces posle redukcije | Globalni indeksi c | Vrednosti |
| --- | --- | --- |
| P3 | 0, 4 | 204, 1356 |
| P7 | 1, 5 | 492, 1644 |
| P11 | 2, 6 | 780, 1932 |
| P15 | 3, 7 | 1068, 2220 |

P15 prikazuje konačan rezultat:

```text
Result process: 15
Maximum A: 64
Vector C: 204 492 780 1068 1356 1644 1932 2220
```

## Pokretanje i provera

Iz ovog foldera:

```sh
mpicc -std=c99 -Wall -Wextra mpi_cyclic_matrix_vector_max.c -o mpi_cyclic_matrix_vector_max
mpirun --oversubscribe -np 16 ./mpi_cyclic_matrix_vector_max
```

Program koristi postojeći `mpi_helpers.h` za prikaz početnih podataka i lokalne raspodele. Dodatne grupne operacije iz pomoćnog prikaza služe samo ispisu.

Kompajliranje prolazi bez upozorenja. Izvršavanja sa 1, 4, 16 i 64 procesa daju očekivani sekvencijalni proizvod. Dodatna privremena varijanta sa `a[6][5] = 100` proverena je sa 4 i 16 procesa: odredišta su P1 i P9, a `c[6] = 2208`. Time je proveren i koren koji nije poslednji proces.

## Lokalni izvori

- `../exam_2023_10_02/mpi_block_matrix_vector_file.c`: ista ciklična raspodela, raspodela b i sabiranje doprinosa.
- `../exam_2020_07_03/mpi_block_matrix_vector_a.c`: slanje jednog izvedenog tipa i sklapanje rezultata kod vlasnika ekstremuma; ovde su prilagođeni ciklične vrste, maksimum i prijem c.
- `01 - Current Accreditation/04 - Additional Materials/Paralelni sistemi/elfak-parallel-systems-main/Lab vežbe/Lab1/zad2/Source.cpp`: primer redukcije maksimuma i ranga pomoću `MPI_MAXLOC`.
- Proveren je i folder `01 - Current Accreditation/04 - Additional Materials/Paralelni sistemi`, uključujući primere u `elfak-parallel-systems-main/Priprema/MPIFiles`. Primer `okt2-2021/Source.c` obrađuje drugi zadatak, sa MPI fajlovima; nije korišćen kao rešenje ovog zadatka.

Skica je izrađena ugrađenim alatom imagegen po uzoru na postojeće skice iz 2020. i 2022. Prompt je u [matrix_vector_sketch_prompt.txt](matrix_vector_sketch_prompt.txt).
