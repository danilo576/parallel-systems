# Množenje matrica sa cikličnom raspodelom

Prvi kolokvijum iz Paralelnih sistema, 25. april 2023, zadatak 1. Tekst zadatka je u fajlu [mpi_cyclic_matrix_multiplication.c](mpi_cyclic_matrix_multiplication.c), iznad funkcije `main`.

## Postupak

1. Proces P0 inicijalizuje matrice A (k × m) i B (m × l). Ako je p broj procesa, svaki proces dobija q = m/p kolona A i isti broj vrsta B. Proces Pi dobija indekse i, i + p, …, i + (q − 1)p.
2. Za svaku matricu koristi se jedan `MPI_Scatter` sa korenom 0. Za A se koristi `MPI_Type_vector(k * q, 1, p, MPI_DOUBLE, ...)`. Pošto je m deljivo sa p, korak p kroz linearnu memoriju bira iste pozicije u svakoj vrsti. `MPI_Type_create_resized` postavlja extent na veličinu jednog elementa, pa blok za Pi počinje u koloni i. Za B se koristi `MPI_Type_vector(q, l, p * l, MPI_DOUBLE, ...)`. Extent je veličina jedne vrste, pa blok za Pi počinje u vrsti i. Oba lokalna bloka primaju se kao uzastopni elementi.
3. Svaki proces množi svoja dva bloka. Za element rezultata C[r][s], proces Pi sabira proizvode A[r][i + t·p] · B[i + t·p][s] za t od 0 do q − 1. Svaki lokalni proizvod ima dimenzije k × l.
4. Procesi pronalaze minimum svojih blokova A i šalju parove (minimum, rang) operacijom `MPI_Reduce` sa `MPI_MINLOC` i korenom 0. Ako više procesa sadrži globalni minimum, bira se najmanji rang. P0 prosleđuje izabrani rang svima pomoću `MPI_Bcast`.
5. `MPI_Reduce` sa `MPI_SUM` sabira lokalne proizvode element po element, direktno u procesu sa izabranim rangom. Taj proces ispisuje C. Sve kolektivne operacije koriste `MPI_COMM_WORLD`.

## Primer sa dva procesa

P0 dobija kolone A i vrste B sa indeksima 0 i 2, pa je njegov doprinos za C[0][0] jednak 2·1 + 4·1 = 6. P1 dobija indekse 1 i 3, pa je njegov doprinos za isti element 1·0 + 3·0 = 0. Sabiranjem se dobija C[0][0] = 6. P1 sadrži minimum A[0][1] = 1 i ispisuje matricu C sa vrstama `6 4` i `8 10`.

![Ciklična raspodela, lokalni proizvodi i sabiranje na procesu P1](distribution_sketch.png)

[Prompt za skicu](distribution_sketch_prompt.txt)

## Pokretanje

```sh
mpicc -std=c11 -Wall -Wextra mpi_cyclic_matrix_multiplication.c -o mpi_cyclic_matrix_multiplication
mpirun -np 2 ./mpi_cyclic_matrix_multiplication
```
