# Množenje matrice i vektora — 24.09.2021.

[Rešenje u C-u](mpi_cyclic_matrix_vector.c)

## Zadatak

Napisati MPI program za množenje matrice A dimenzija n × n i vektora b dužine n, čime se dobija vektor c. A i b inicijalizuje master proces P0. Ukupno p = q² procesa uređeno je u mrežu q × q. Matrica se deli ciklično po vrstama i kolonama: proces P(r·q+t) dobija elemente A[r+i·q][t+j·q], za i,j = 0,…,n/q−1, kao na priloženoj slici za n=8 i p=16. Proces Pi dobija b[i%q], b[i%q+q], …, b[i%q+n−q].

Sve vrednosti lokalnog bloka A šalju se procesu odjednom. Svi procesi, uključujući master, računaju i učestvuju u generisanju rezultata. Rezultat se prikazuje u procesu koji sadrži minimum matrice A. Raspodelu A i b i generisanje rezultata realizovati grupnim operacijama i novim komunikatorima.

## Postupak

1. Proveravamo da je p=q² i da je N deljivo sa q. Oznake su `row=rank/q`, `col=rank%q`, `s=N/q`. Važi `local_a[i][j]=A[row+i*q][col+j*q]` i `local_b[j]=b[col+j*q]`.
2. `MPI_Comm_split` pravi `row_comm` sa lokalnim rangom `col` i `col_comm` sa lokalnim rangom `row`. Lokalni rang korena zato nije uvek jednak njegovom globalnom rangu.
3. Prvi `MPI_Scatter`, samo u koloni 0, deli ciklične vrste A. Tip `strip_type` opisuje s blokova dužine N sa korakom q·N. Njegov extent postavljamo na N elemenata, pa sledeći proces dobija sledeću početnu vrstu. P0, P4, P8 i P12 za q=4 dobijaju vrste {0,4}, {1,5}, {2,6} i {3,7}. Svaka primljena traka `strip[s][N]` lokalno je kontinualna.
4. Tip `cyclic_type` opisuje s elemenata sa korakom q. `MPI_Type_vector(rows * cols, 1, q, MPI_DOUBLE, &block_type)` bira svih s² elemenata lokalnog bloka, sa korakom q kroz kontinualnu traku. Pošto važi N=cols·q, isti korak prelazi i sa poslednjeg izabranog elementa jedne vrste na prvi izabrani element sledeće vrste. Extent završnog `block_resized` je jedan element. Drugi `MPI_Scatter`, u svakoj vrsti mreže, zato daje procesu iz kolone t upravo kolone t,t+q,… cele trake. Svaki proces prima svih s² elemenata svog bloka u jednom pozivu. Ne raspodeljujemo pojedinačne elemente zasebnim porukama.
5. `MPI_Scatter` u prvoj vrsti mreže deli b istim cikličnim tipom, čiji je extent jedan element. `MPI_Bcast` zatim prosleđuje dobijeni deo b niz odgovarajuću kolonu, sa lokalnim korenom 0. Na primer, P1, P5, P9 i P13 svi dobijaju b[1] i b[5].
6. Svaki proces računa `local_c[i] = Σj local_a[i][j]*local_b[j]`. To je doprinos za globalni element c[row+i·q], a ne još ceo rezultat. Istovremeno nalazi minimum svog bloka.
7. `MPI_Reduce` sa `MPI_MINLOC` nad parom (minimum, globalni rang), pa `MPI_Bcast`, određuju proces koji prikazuje rezultat. Kod jednakih minimuma bira se najmanji globalni rang među procesima koji ih sadrže.
8. `MPI_Reduce` sa `MPI_SUM` po svakoj vrsti mreže sabira doprinose za iste globalne indekse c. Lokalni koren je `result_col = global_min.rank%q`.
9. Samo izabrana kolona poziva `MPI_Gather`, sa korenom `result_row = global_min.rank/q`. Prijemni ciklični tip smešta prilog vrste r direktno na pozicije c[r], c[r+q], … . Time se dobija prirodan redosled vektora bez naknadnog preslaganja. Za N=8,q=4 to su parovi {0,4}, {1,5}, {2,6}, {3,7}.

`MPI_Type_create_resized` menja razmak početaka priloga u Scatter/Gather operacijama; ne menja izabrane elemente tipa. Svi koreni učestvuju i svojim podacima, a dodatna sinhronizacija nije potrebna.

## Konkretan primer

Inicijalizacija je A[i][j]=8·i+j+1, osim A[7][7]=−1, i b[j]=j+1. Za 16 procesa minimum je u P15.

P1 ima vrste {0,4} i kolone {1,5}:

```text
local_a = [[2, 6], [34, 38]]
local_b = [2, 6]
local_c = [40, 296]
```

Prva vrsta mreže sabira:

```text
P0 [26,218] + P1 [40,296] + P2 [58,378] + P3 [80,464]
= [204,1356] = [c[0],c[4]]
```

Posle Reduce operacija P3 drži [204,1356], P7 [492,1644], P11 [780,1932], a P15 [1068,1700]. Gather u P15 slaže:

```text
Result process: 15
Minimum A: -1
Vector C: 204 492 780 1068 1356 1644 1932 1700
```

## Skica

![Ciklična raspodela matrice i vektora i komunikacije](matrix_vector_sketch.png)

[Prompt za skicu](matrix_vector_sketch_prompt.txt). Skica je generisana ugrađenim imagegen alatom, uz postojeću skicu kao stilsku referencu. Strelice predstavljaju logički tok grupnih operacija.

## Prevođenje i provera

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_cyclic_matrix_vector.c -o mpi_cyclic_matrix_vector
mpirun --oversubscribe -np 16 ./mpi_cyclic_matrix_vector
```

N je podrazumevano 8 i može se promeniti opcijom `-DN=12`. Koriste se lokalni nizovi promenljive dužine, kao u susednim primerima; rešenje je namenjeno malim ispitnim matricama.

Prevođenje prolazi bez upozorenja. Za p=1,4,16 svi elementi rezultata provereni su nezavisnim sekvencijalnim množenjem, kao i izbor procesa za ispis. Za p=2 program ispravno odbija nevažeću mrežu.

## Lokalni izvori

- [02.07.2021, varijanta b](../exam_2021_07_02/mpi_cyclic_rows_matrix_vector_b.c): raspodela u dva Scatter koraka, komunikatori, MINLOC i završni Reduce/Gather. Ovde su i kolone ciklične, pa je prilagođen tip lokalnog bloka i raspodela b.
- [03.07.2020, skica](../exam_2020_07_03/matrix_vector_sketch.png): stilska referenca.
- [23.06.2022, objašnjenje](../exam_2022_06_23/README.md): izvedeni tipovi i grupne operacije kroz mrežu.
- Provereni su i dopunski materijali `04 - Additional Materials/Paralelni sistemi`; fajl `Ispit/21septembar3.c` obrađuje MPI-IO i ne odgovara ovom zadatku. Raspodela za ovaj datum izvedena je iz dostavljene slike zadatka.
