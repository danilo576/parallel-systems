# Ispit 20.06.2024. — zadatak 1

Svi procesi čitaju delove niza iz `niz.dat`. Proces koji pročita najveći element računa i ispisuje rezultat `M`.

## Postupak

1. Svaki proces određuje broj elemenata iz veličine datoteke i čita `N/p` uzastopnih vrednosti pozivom `MPI_File_read_shared`. Zajednički pokazivač se pomera posle svakog čitanja, pa svaki blok pripadne tačno jednom procesu. Redosled dodele blokova zavisi od redosleda poziva.
2. Svaki proces računa lokalni zbir i maksimum. Na procesu 0, redukcija sa `MPI_SUM` daje zbir svih elemenata, pa proces 0 računa `As = zbir / N`. Redukcija sa `MPI_MAXLOC` i tipom `MPI_DOUBLE_INT` daje rang vlasnika maksimuma. Ako je maksimum jednak kod više procesa, bira se najmanji rang.
3. Proces 0 pomoću dva poziva `MPI_Bcast` prosleđuje vrednosti `As` i `C`, kao i odredišni rang.
4. Svaki proces za svaki svoj element računa `(A[i] + As) / C` i sabira te članove. Završni `MPI_Reduce` sa `MPI_SUM` prenosi zbir lokalnih rezultata direktno vlasniku maksimuma, koji ispisuje `M`. Sve kolektivne operacije koriste `MPI_COMM_WORLD`.

![Primer čitanja, računanja srednje vrednosti i sabiranja](array_sum_direct_sketch.png)

Za prikazanu raspodelu srednja vrednost je 3. Proces 0 daje doprinos 7, a proces 1 doprinos 5. Proces 0 prima zbir 12 jer je pročitao maksimum 6.

## Pokretanje

Format datoteke je binarni niz vrednosti tipa `double` u izvornom formatu računara, bez zaglavlja. Proces 0 u kodu postavlja `C = 2.0`.

```sh
python3 -c 'import struct; from pathlib import Path; Path("niz.dat").write_bytes(struct.pack("@4d", 2, 6, 1, 3))'
mpicc -std=c11 -Wall -Wextra mpi_shared_array_sum.c -o mpi_shared_array_sum
mpirun -np 2 ./mpi_shared_array_sum
```

Očekivana vrednost je `M = 12`. Ispisani rang može biti 0 ili 1.
