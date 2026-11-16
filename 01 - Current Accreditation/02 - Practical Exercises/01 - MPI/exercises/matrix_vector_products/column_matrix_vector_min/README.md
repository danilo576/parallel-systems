# Samostalan rad: proizvod matrice i vektora i minimumi vrsta

Otvori `mpi_column_matrix_vector_min.c`. Tekst zadatka je iznad `main`.
Pripremljeni su MPI inicijalizacija, provera broja procesa i ulazni podaci.
Delovi koje samostalno implementiraš označeni su sa `TODO`. Kostur se kompajlira,
ali još ne računa rezultate.

## Postavka

- Matrica A ima M = 4 vrste i K = 8 kolona.
- Vektor b = [1, 2, 3, 4, 5, 6, 7, 8] inicijalizovan je u svim procesima.
- S = 2 je zadata konstanta: pokreni 4 procesa, tako da važi K = S * p.
- Root je P0. Pošto tekst ne određuje proces za ispis, rezultate prikaži u P0.
- Koristi `double` i odgovarajući MPI osnovni tip.
- Ulazni fajl i generator nisu potrebni za ovaj zadatak.

| Proces | Kolone A (indeksi od nule) |
| --- | --- |
| P0 | 0, 1 |
| P1 | 2, 3 |
| P2 | 4, 5 |
| P3 | 6, 7 |

## Redosled rada

1. Na papiru skiciraj matricu i označi blok svakog procesa.
2. Odredi lokalne nizove i njihove dimenzije.
3. Opiši blok kolona izvedenim tipom. Razmisli o rasporedu matrice u C memoriji i razmaku između početaka blokova.
4. Raspodeli podatke grupnom operacijom, direktno iz A, bez pomoćnog pakovanja kolona.
5. Iz primljenih podataka izračunaj doprinos proizvodu i minimum za svaku vrstu. U ovom delu mora učestvovati i P0.
6. Grupnim operacijama objedini lokalne rezultate i ispiši oba vektora.
7. Oslobodi kreirane MPI tipove.

Proveri da lokalni indeks kolone povezuješ sa odgovarajućim elementom b.
Minimum se računa posle raspodele, iz lokalnih blokova. Razmisli kako inicijalizuješ
minimum: ulaz sadrži i pozitivne i negativne vrste.

## Pokretanje

Otvori C fajl, pokreni **Build and run MPI or OpenMP** i unesi `4`.
Ili iz ovog foldera:

```sh
mpicc mpi_column_matrix_vector_min.c -o mpi_column_matrix_vector_min
mpirun --oversubscribe -np 4 ./mpi_column_matrix_vector_min
```

Za dodatnu proveru promeni S na 4 i pokreni 2 procesa, zatim S na 8 i pokreni
1 proces. Rezultati moraju biti isti. Vrati S na 2 za početnu postavku.

## Provera kada završiš

<details>
<summary>Otvori očekivane rezultate</summary>

Za inicijalizovane podatke:

```text
C = [179, 168, 106, 156]
Row minima = [1, -3, -4, -2]
```

Očekivani minimumi namerno se nalaze u različitim blokovima kolona.
U rezultatu treba da postoje četiri minimuma, po jedan za svaku vrstu.

</details>

## Lokalni materijali za podsećanje

- `../../../materials/Izvedeni tipovi podataka.pdf`
- `../../../materials/MPI-grupne operacije.pdf`
- `../../../materials/mpi_function_reference.pdf`
- Dodatno je pregledan lokalni primer `elfak-parallel-systems-main/Lab vežbe/Lab1/zad1/Source.cpp` iz `04 - Additional Materials/Paralelni sistemi`: primer raspodele matrice i nalaženja minimuma. To je drugi zadatak, pa ga koristi samo kao referencu.

Prethodni zadatak u `../column_matrix_vector_product/` sadrži gotovo rešenje
sličnog proizvoda; ako želiš potpuno samostalan pokušaj, ostavi ga zatvorenog.
