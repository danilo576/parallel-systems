# Virtuelna topologija — 25.04.2023, zadatak 2

Procesi u dvodimenzionalnoj Kartezijanskoj mreži razmenjuju lokalnu vrednost `K` sa susedima iz iste vrste i kolona iste parnosti. Proces zatim uvećava primljenu vrednost za svoj rang. Samo procesi u parnim kolonama, numerisanim od nule, ispisuju `K` pre i posle razmene.

![Mreža 2 × 4, razmena među kolonama iste parnosti i vrednosti K](topology_sketch.png)

## Postupak

1. Pravimo mrežu `n × m` pomoću `MPI_Cart_create`. Biramo periodične kolone kako bi se razmena nastavila preko ivice vrste. Redosled rangova ostaje isti kao u `MPI_COMM_WORLD`.
2. Svaki proces postavlja `K` na svoj rang. Pozivom `MPI_Cart_shift` u drugoj dimenziji sa pomakom `+2` dobija izvor iz kolone `(c − 2) mod m` i odredište u koloni `(c + 2) mod m`. Kako je `m` parno, pomak čuva parnost i pri prelasku preko ivice.
3. Pomoću `MPI_Sendrecv_replace` svaki proces šalje početni `K` odredištu i u istu promenljivu prima vrednost od izvora. Zatim dodaje svoj rang. U mreži sa četiri kolone izvor i odredište su isti sused, pa su strelice na skici dvosmerne. Sa dve kolone svaki proces razmenjuje podatak sa samim sobom.

U mreži `2 × 4` proces P6 iz kolone 2 prima `K = 4` od P4 iz kolone 0 i dobija `K = 4 + 6 = 10`. Konačne vrednosti koje se ispisuju jesu P0 i P2 po 2, a P4 i P6 po 10. Redosled ispisa između procesa zavisi od izvršavanja.

## Pokretanje

Iz ovog foldera pokrenuti sledeće komande. Argumenti programa su broj vrsta i broj kolona.

```sh
mpicc -std=c11 -Wall -Wextra mpi_parity_exchange.c -o mpi_parity_exchange
mpirun -np 8 ./mpi_parity_exchange 2 4
```

Lokalni izvori su [prezentacija o virtuelnim topologijama](../../../materials/Virtuelne%20topologije.pdf) i primer `Zad2.c` iz `Priprema/MPI/kol-2023-treca-god` u dopunskim materijalima.
