# MPI — Direktan prenos elemenata matrice (19.06.2017, 4a)

P0 ima matricu `A` dimenzija `n × n`, gde je `n` parno. Svaki proces sa rangom različitim od nule formira svoju matricu `C` istih dimenzija. P0 šalje po jednu poruku svakom primaocu.

![Izbor elemenata matrice A, jedna poruka po primaocu i raspored u matrici C](distribution_sketch.svg)

## Postupak

1. U matrici `A` obilazimo parove kolona `(0, 1), (2, 3), …` sleva nadesno. Za svaki par uzimamo dva susedna elementa iz svake parne vrste, odozgo nadole. Za `n = 6` redosled počinje sa `a00, a01, a20, a21, a40, a41, a02, a03`.
2. Pošiljalac pomoću `MPI_Type_indexed` opisuje `(n/2)²` blokova dužine `2`. Pomeraj svakog bloka je `row * n + col`, izražen u elementima tipa `MPI_INT`. Jedan `MPI_Send` po primaocu čita elemente direktno iz `A`.
3. Primalac inicijalizuje `C` nulama. Obilazi kolone zdesna nalevo, a u svakoj koloni ide odozdo nagore dok važi `row + col >= n - 1`. Uzima prvih `n²/2` pozicija. Za `n = 6` redosled počinje sa `C[5][5], C[4][5], C[3][5], C[2][5], C[1][5], C[0][5], C[5][4], C[4][4]` i završava se na `C[3][2]`.
4. Prijemni `MPI_Type_indexed` ima `n²/2` blokova dužine `1`. Oba tipa opisuju isti broj elemenata tipa `MPI_INT`. Jedan `MPI_Recv` upisuje elemente direktno na mesta u `C` redom kojim su poslati.

Obrazac obilaska za opšte parno `n` prati lokalno rešenje `Danica 19.6.2017..txt` iz dopunskih materijala. U programu je `A[row][col] = row * n + col + 1` da bi se po ispisu videlo odakle dolazi svaki element.

## Pokretanje

```sh
mpicc -std=c11 -Wall -Wextra mpi_matrix_redistribution.c -o mpi_matrix_redistribution
mpirun -np 3 ./mpi_matrix_redistribution
```

Za drugu parnu dimenziju dodaj opciju `-DN=8` pri prevođenju.
