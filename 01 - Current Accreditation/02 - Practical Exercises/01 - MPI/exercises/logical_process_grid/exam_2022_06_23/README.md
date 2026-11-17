# Množenje matrica — 23.06.2022.

[Rešenje u C-u](mpi_block_matrix_product.c)

## Zadatak

Matrice A i B dimenzija n × n inicijalizuju se u master procesu P0. Ukupno p = q² procesa uređeno je u mrežu q × q. Matrice se dele na blokove k × k, gde je k = n/q. Proces P(rq+s) dobija blok sa vrstama rk…(r+1)k−1 i kolonama sk…(s+1)k−1 iz obe matrice. Svaki blok se šalje odjednom.

Posle početne raspodele svaki proces treba da dobije sve blokove A iz svoje vrste procesa i sve blokove B iz svoje kolone procesa. Zatim izračunava svoj deo C = A·B. Rezultat se prikazuje u P0. Point-to-point operacije dozvoljene su samo za početnu raspodelu; dalja komunikacija i prikupljanje rezultata koriste grupne operacije i nove komunikatore.

## Postupak

1. Odredimo `q`, proverimo `p = q*q` i deljivost N sa q. Koordinate procesa su `row = rank/q`, `col = rank%q`, a veličina bloka `k = N/q`.
2. `MPI_Type_vector(k, k, N, MPI_DOUBLE, &block_type)` opisuje k vrsta od po k elemenata sa razmakom N između početaka susednih vrsta. Jedan `MPI_Send` šalje ceo blok A, drugi ceo blok B. Prijem je u kontinualne lokalne matrice k × k. P0 svoj blok kopira.
3. `MPI_Comm_split(MCW, row, col, &row_comm)` grupiše istu vrstu i uređuje procese po koloni. `MPI_Comm_split(MCW, col, row, &col_comm)` grupiše istu kolonu i uređuje procese po vrsti.
4. `MPI_Gather` u `row_comm` prikuplja blokove A u `a_row_strip[k][N]` procesa iz kolone 0. Prijemni tip `row_strip_block_type` izveden je iz `block_type`, uz extent od `k*sizeof(double)`. Tako svaki sledeći blok počinje k elemenata desno, a njegove vrste imaju korak N. Na primer, za k = 2, N = 8, prvi blok zauzima pozicije 0, 1, 8, 9, a drugi 2, 3, 10, 11. `MPI_Bcast` zatim šalje svih k·N elemenata celoj vrsti procesa.
5. `MPI_Gather` u `col_comm` prikuplja blokove B u `b_column_strip[N][k]` procesa iz vrste 0. Ovde običan prijem po k·k elemenata već daje ispravan raspored: blokovi se nadovezuju po vrstama. `MPI_Bcast` šalje svih N·k elemenata celoj koloni procesa.
6. Svaki proces računa svoj blok `local_c_block[k][k]` običnim množenjem matrica:

   `local_c_block[i][j] = Σ(t=0..N−1) a_row_strip[i][t] * b_column_strip[t][j]`.

   Na primer, P6 koristi vrste 2–3 matrice A i kolone 4–5 matrice B, pa računa vrste 2–3 i kolone 4–5 matrice C. Niz `local_c_block` sadrži ceo završni blok C tog procesa.
7. Prvi završni `MPI_Gather`, u `col_comm`, spaja blokove C po vertikali. Procesi iz vrste 0 dobijaju po jednu celu traku `c_column_strip[N][k]`.
8. Samo procesi za koje je `row == 0` pozivaju drugi završni `MPI_Gather`, u svom `row_comm`. Tip `resized_column_strip_type` opisuje N vrsta sa po k elemenata i korakom N, uz extent k elemenata. Time se trake direktno smeštaju jedna pored druge u `c[N][N]` procesa P0, bez naknadnog preslaganja.

Ovo prati strukturu tvog rešenja. Izvedeni prijemni tip potreban je za horizontalno slaganje blokova A i završnih traka C. Za vertikalno slaganje blokova B i C dovoljan je običan `MPI_DOUBLE`. `MPI_Type_create_resized` menja razmak između početaka priloga procesa u Gather-u; ne menja elemente opisane izvornim tipom.

## Skica

![Raspodela matrica, mreža procesa i komunikacije](matrix_product_sketch.png)

Svaki proces ima posebnu boju, doslednu u obe matrice i mreži procesa. Oznake P0–P15 predstavljaju vlasnike blokova, a indeksi uz matrice označavaju vrste i kolone. Strelice u mreži označavaju grupe razmene, a ne redosled pojedinačnih slanja pri operacijama Gather i Bcast. Završno prikupljanje u kodu obavlja se u dva koraka: Gather po kolonama, pa Gather samo u prvoj vrsti procesa.

## Prevođenje i pokretanje

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_block_matrix_product.c -o mpi_block_matrix_product
mpirun --oversubscribe -np 16 ./mpi_block_matrix_product
```

Podrazumevano je N = 8. Podržani su, na primer, p = 1, 4 i 16. Za drugu dimenziju upotrebiti `-DN=12` pri prevođenju; N mora biti pozitivan i deljiv sa q. Rešenje koristi lokalne nizove promenljive dužine kao susedni primeri i namenjeno je malim ispitnim matricama.

## Lokalni izvori

- [Rešenje jun 2022. u dopunskim materijalima](../../../../../04%20-%20Additional%20Materials/Paralelni%20sistemi/elfak-parallel-systems-main/Priprema/MPI/jun-2022/Source.c): blokovski tip i komunikatori vrsta/kolona. Tamo završno prikupljanje koristi Send/Recv; ovde je zamenjeno zahtevanom grupnom operacijom Gather.
- [Primer 12.06.2019.](../exam_2019_06_12/mpi_block_matrix_vector.c): raspodela blokova pomoću izvedenog tipa i formiranje komunikatora.

## Provera

Prevođenje sa `-std=c99 -Wall -Wextra -pedantic` prolazi bez upozorenja. Za p = 1, 4 i 16 svih 64 elemenata rezultata poklapa se sa nezavisnim sekvencijalnim množenjem. Za p = 2 program ispravno odbija nevažeću mrežu procesa. Opcija `--oversubscribe` u Open MPI omogućava pokretanje 16 procesa i kada računar ima manje raspoloživih procesorskih jezgara.
