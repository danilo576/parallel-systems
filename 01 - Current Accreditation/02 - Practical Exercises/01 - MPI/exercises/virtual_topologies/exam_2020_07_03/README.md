# Kartezijanska topologija — 3. 7. 2020, zadatak 4(a)

U mreži 3 × 4 vrednosti se ciklično pomeraju udesno za 0, 1 i 2 mesta u vrstama 0, 1 i 2.

1. Neka su `r` i `c` koordinata vrste i kolone procesa. Početna vrednost je `x = r + c`.
2. `MPI_Cart_create` formira mrežu sa dimenzijama `{3, 4}` i periodičnošću `{0, 1}`. Zato se pomak po kolonama nastavlja od početka iste vrste.
3. `MPI_Cart_shift(cartcomm, 1, r, &source, &dest)` računa rangove u Kartezijanskom komunikatoru. Proces šalje u kolonu `(c + r) % 4` i prima iz kolone `(c - r + 4) % 4`. Na primer, P4 na koordinatama (1, 0) šalje svoju jedinicu P5 i prima četvorku od P7.
4. `MPI_Sendrecv_replace` šalje početni sadržaj `x` i upisuje primljenu vrednost u istu promenljivu. U vrsti 0 procesi šalju sami sebi.

| Vrsta | x pre | x posle |
| --- | --- | --- |
| 0 | 0 1 2 3 | 0 1 2 3 |
| 1 | 1 2 3 4 | 4 1 2 3 |
| 2 | 2 3 4 5 | 4 5 2 3 |

![Kartezijanska mreža, vrednosti pre i posle pomaka i strelice razmene](topology_sketch.png)

[Prompt za skicu](topology_sketch_prompt.txt)

## Pokretanje

```sh
mpicc -std=c99 -Wall -Wextra mpi_cartesian_row_shift.c -o mpi_cartesian_row_shift
mpirun --oversubscribe -np 12 ./mpi_cartesian_row_shift
```

Program ispisuje rang, koordinate i vrednost pre i posle razmene za svaki proces. Redosled ispisa zavisi od izvršavanja procesa.

Obrazac za formiranje topologije i razmenu nalazi se i u [zadatku od 25. 4. 2023.](../exam_2023_04_25/mpi_parity_exchange.c).
