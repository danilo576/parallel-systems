# Ciklično množenje matrice i vektora — 07.07.2022, zadatak 2a

[Rešenje u C-u](mpi_cyclic_matrix_vector.c) · [Zadatak 2b](../../logical_process_grid/exam_2022_07_07/README.md)

## Postupak

1. P0 inicijalizuje matricu A dimenzija M × N i vektor b dužine N. Za p procesa, uz uslov da p deli N, svaki proces dobija s = N/p kolona i isto toliko elemenata vektora.
2. Za matricu `MPI_Type_vector(M*s, 1, p, MPI_INT, ...)` bira svaki p-ti element pri linearnom prolazu kroz matricu. Pošto je N = p·s, posle s elemenata prelazi se na istu početnu kolonu naredne vrste. Za b analogni tip bira s elemenata.
3. `MPI_Type_create_resized` postavlja extent oba tipa na veličinu jednog elementa. Dva poziva `MPI_Scatter`, jedan za A i jedan za b, tako počinju blok za Pᵢ na pomeraju i. Blok matrice se prima kao uzastopnih M·s elemenata, uređeno po vrstama.
4. Svaki proces za svaku vrstu r računa zbir proizvoda A[r][i + k·p] · b[i + k·p] za k = 0, …, s−1. `MPI_Reduce` sa `MPI_SUM` sabira svih M komponenti po indeksima i rezultat smešta u P0.

Za p = 2, P0 koristi kolone 0 i 2, a P1 koristi kolone 1 i 3. Njihovi doprinosi su [10, 26] i [20, 44], pa je c = [30, 70].

## Skica

![Ciklična raspodela matrice i vektora, dva Scatter poziva i sabiranje u P0](matrix_vector_sketch.png)

[Prompt za skicu](matrix_vector_sketch_prompt.txt)

## Prevođenje i pokretanje

```sh
mpicc -std=c99 -Wall -Wextra -pedantic mpi_cyclic_matrix_vector.c -o mpi_cyclic_matrix_vector
mpirun -np 2 ./mpi_cyclic_matrix_vector
```

Za 1, 2 i 4 procesa provereno je da program ispisuje isti rezultat. Pokretanje sa 3 procesa odbija se jer 3 ne deli N = 4.

## Lokalni izvori

- [Rešenje iz dopunskih materijala](../../../../../04%20-%20Additional%20Materials/Paralelni%20sistemi/elfak-parallel-systems-main/Priprema/MPI/jun2-2022/Zad1.cpp)
- [Primer množenja matrice i vektora po kolonama](../column_matrix_vector_product/mpi_column_matrix_vector_product.c)
