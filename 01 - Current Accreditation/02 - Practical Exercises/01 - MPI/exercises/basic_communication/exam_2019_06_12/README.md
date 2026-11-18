# MPI — Prosleđivanje niza zaposlenih (12.06.2019, 4a)

Proces ranga 0 učitava niz zaposlenih sa tastature. Posle komunikacije svaki proces u `MPI_COMM_WORLD` ima ceo niz u istom redosledu.

![Prosleđivanje broja zaposlenih i celog niza iz ranga 0](broadcast_sketch.png)

## Postupak

1. Rang 0 učitava pozitivan broj zaposlenih i prosleđuje ga prvim pozivom `MPI_Bcast`. Svaki proces zatim rezerviše prostor za niz.
2. Rang 0 učitava matični broj, ime, prezime i prosečnu platu za svakog zaposlenog. Ime i prezime unose se kao zasebne reči, svaka do 31 bajta.
3. Svaki proces pomoću `MPI_Type_create_struct` opisuje jedan zapis kao četiri bloka dužina `1, 32, 32, 1` sa tipovima `MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT`. `MPI_Get_address` daje adresu prvog zapisa i adrese njegovih polja. `MPI_Aint_diff` računa pomeraj svakog polja kao razliku između njegove adrese i adrese početka zapisa. `MPI_Type_create_resized` postavlja razmak između uzastopnih zapisa na `sizeof(Employee)`, uključujući eventualnu dopunu na kraju strukture. `MPI_Type_commit` priprema izvedeni tip za komunikaciju.
4. Jedan poziv `MPI_Bcast(employees, count, employee_type, 0, MPI_COMM_WORLD)` prosleđuje sve zapise. Svaki proces ispisuje podatke koje ima, a redosled ispisa između procesa može se razlikovati.

## Pokretanje

```sh
mpicc -std=c11 -Wall -Wextra mpi_employee_broadcast.c -o mpi_employee_broadcast
mpiexec -n 3 ./mpi_employee_broadcast
```

Primer unosa na procesu ranga 0

```text
2
101 Ana Ilic 85000
102 Marko Jovic 92000
```

## Lokalni izvori

- [Izvedeni tipovi podataka](../../../materials/Izvedeni%20tipovi%20podataka.pdf)
- [MPI grupne operacije](../../../materials/MPI-grupne%20operacije.pdf)
