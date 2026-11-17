# Zadatak 3 — 18.01.2021.

Napisati MPI program za množenje matrica A i B dimenzija n × n. Matrice inicijalizuje master proces. Procesi su uređeni kao mreža q × q, gde je p = q². Matrica A deli se na trake od n/q vrsta, a B na trake od n/q kolona. Svi procesi iste vrste mreže dobijaju istu traku A, a svi procesi iste kolone istu traku B. Procesi računaju rezultat C koji se prikazuje u master procesu. Za slanje blokova koristiti izvedene tipove, a za raspodelu i sklapanje rezultata grupne operacije i nove komunikatore.

Rešenje: [mpi_strip_matrix_product.c](mpi_strip_matrix_product.c).

## Postupak

1. Odredimo q i proverimo da je p = q² i da je N deljivo sa q. Proces ima koordinate `row = rank/q`, `col = rank%q`; širina trake je `k = N/q`.
2. `MPI_Comm_split(MCW, row, col, &row_comm)` povezuje procese iste vrste, uređene po koloni. Zato je lokalni rang u njemu jednak `col`. Drugi poziv, sa `col, row`, formira `col_comm`, u kome je lokalni rang jednak `row`.
3. Traka A ima k celih uzastopnih vrsta, odnosno k·N uzastopnih elemenata. Opisuje je `MPI_Type_contiguous(k*N, MPI_DOUBLE, ...)`. Samo procesi sa `col == 0` učestvuju u početnom `MPI_Scatter` u svom `col_comm`, sa korenom 0 (P0). Proces P(row·q) dobija vrste od row·k do (row+1)·k−1. Zatim `MPI_Bcast` u svakom `row_comm`, sa lokalnim korenom 0, kopira traku svim procesima te vrste.
4. Traku B opisuje `MPI_Type_vector(N, k, N, MPI_DOUBLE, ...)`: N grupa od k susednih elemenata, sa korakom N. `MPI_Type_create_resized` postavlja extent na k·sizeof(double), tako da naredni prilog u Scatter-u počinje k kolona desno. Samo procesi sa `row == 0` pozivaju `MPI_Scatter` u svom `row_comm`, sa korenom 0 (P0). Prijem u `b_column_strip[N][k]` je kontinualan. Potom `MPI_Bcast` u svakom `col_comm`, sa lokalnim korenom 0, kopira traku niz kolonu mreže.
5. Svaki proces računa blok `local_c_block[k][k]`:

   `local_c_block[i][j] = Σ(t=0..N−1) a_row_strip[i][t] * b_column_strip[t][j]`.

   Globalni element je `C[row*k+i][col*k+j]`. Svaki proces već ima sve N činilaca za svaki svoj element, pa nije potreban Reduce.
6. `MPI_Gather` u svakom `col_comm`, sa lokalnim korenom 0, nadovezuje k × k blokove po vrstama. Procesi u prvoj vrsti mreže dobijaju po jednu završenu N × k traku C.
7. Samo procesi sa `row == 0` pozivaju završni `MPI_Gather` u svom `row_comm`, sa korenom 0 (P0). Prijemni tip je isti prilagođeni tip kolonske trake kao za B: trake se direktno upisuju jedna pored druge u C, bez naknadnog preslaganja.

Uslovni kolektivni pozivi obuhvataju ceo odgovarajući komunikator: svi članovi kolone 0 imaju `col == 0`, a svi članovi vrste 0 imaju `row == 0`. Svi navedeni koreni su **lokalni rangovi** u datom komunikatoru.

## Primer i skica

![Raspodela traka, lokalno množenje i sklapanje C](matrix_product_sketch.png)

Skica koristi N=4, p=4, q=2 i k=2. A je popunjena brojevima 1–16 po vrstama, a B formulom B[i][j]=i+j+1. Dve boje u polju A ili B označavaju da oba procesa čuvaju kopiju tog elementa. Svaki blok C ima jednog vlasnika.

P1 dobija vrste 0–1 matrice A i kolone 2–3 matrice B:

```text
A strip                 B strip       C block
1 2 3 4                 3 4           50  60
5 6 7 8                 4 5          122 148
                        5 6
                        6 7
```

Na primer, C[0][2] = 1·3 + 2·4 + 3·5 + 4·6 = 50. Sabiraju se lokalni proizvodi za t=0…3. P1 šalje završenu desnu traku pri poslednjem Gather-u, a P0 dobija:

```text
 30  40  50  60
 70  96 122 148
110 152 194 236
150 208 266 324
```

## Prevođenje i pokretanje

Iz ovog foldera:

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_strip_matrix_product.c -o mpi_strip_matrix_product
mpirun --oversubscribe -np 4 ./mpi_strip_matrix_product
```

Podrazumevano je N=8. Za primer sa skice dodati `-DN=4` pri prevođenju. Broj procesa mora biti potpun kvadrat, a N pozitivan i deljiv sa q. Rešenje koristi nizove na steku, kao susedni primeri, i namenjeno je malim ispitnim matricama.

## Lokalni izvori i provera

- [23.06.2022.](../exam_2022_06_23/mpi_block_matrix_product.c): komunikatori vrsta/kolona, lokalno množenje i sklapanje rezultata u dva Gather-a. U ovom zadatku početna raspodela odmah šalje cele trake pomoću Scatter-a.
- [Dopunski materijal, jun 2022.](../../../../../04%20-%20Additional%20Materials/Paralelni%20sistemi/elfak-parallel-systems-main/Priprema/MPI/jun-2022/Source.c): izvedeni tipovi i komunikatori za matrično množenje.
- Skice 03.07.2020. i 23.06.2022. korišćene su kao vizuelni uzori.

Prevođenje sa navedenim upozorenjima prolazi bez upozorenja. Za N=8 i p=1,4,16 svih 64 elemenata C provereno je nezavisnim sekvencijalnim množenjem. Za p=2 provereno je odbijanje nevažeće mreže.

