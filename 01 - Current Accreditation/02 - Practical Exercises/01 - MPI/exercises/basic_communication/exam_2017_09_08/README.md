# MPI — Raspodela kvadranata (08.09.2017, 4a)

Proces P0 ima matricu `A` dimenzija `n × n`, gde je `n` parno. P1 dobija gornji desni kvadrant, P2 donji levi, a P3 donji desni. Svaki od njih formira svoju matricu `C` istih dimenzija.

![Raspodela kvadranata i matrica C na P1](distribution_sketch.png)

## Postupak

1. Neka je `h = n/2`. Tip `MPI_Type_vector(h, h, n, MPI_INT, …)` opisuje `h` uzastopnih vrsta kvadranta, sa po `h` elemenata i korakom `n` između početaka vrsta. P0 bira početak kvadranta prema rangu primaoca.
2. P0 postavlja extent tog tipa na nulu pomoću `MPI_Type_create_resized`. Jedan `MPI_Send` sa brojem ponavljanja `2` čita isti kvadrant dvaput. Tako se, na primer, P1 šalje niz `3, 4, 7, 8, 3, 4, 7, 8`.
3. Primalac postavlja sve elemente `C` na nulu. U vrsti `i` prijem počinje od kolone `n - 1 - i` i ide nadesno. Uzima se prvih `2h²` pozicija obilaskom vrsta odozgo nadole, pa poslednji blok može biti kraći od raspoloživog dela vrste. Ovo pravilo za opšte parno `n` prati lokalno rešenje istog zadatka iz dopunskih materijala.
4. `MPI_Type_indexed` opisuje dužine i pomeraje tih blokova u matrici `C`. Jedan `MPI_Recv` upisuje svih `2h²` elemenata direktno na predviđena mesta.

Za `n = 4`, dužine blokova prijemnog tipa su `1, 2, 3, 2`, a pomeraji, izraženi u elementima tipa `MPI_INT`, iznose `3, 6, 9, 12`. Matrica `C` na P1 izgleda ovako.

```text
0 0 0 3
0 0 4 7
0 8 3 4
7 8 0 0
```

## Pokretanje

```sh
mpicc -std=c11 -Wall -Wextra mpi_quadrant_distribution.c -o mpi_quadrant_distribution
mpirun -np 4 ./mpi_quadrant_distribution
```

Za drugu parnu dimenziju pri prevođenju dodaj, na primer, `-DN=6`.
