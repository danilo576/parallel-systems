# MPI-IO — 28.01.2022, zadatak 3

Program čita binarnu datoteku `file1.dat` i preuređuje njene bajtove u `file2.dat`. U zadatku je ulaz veličine 10 MB. Program očitava stvarnu veličinu datoteke, pa se raspored može proveriti i na malom primeru.

## Postupak

1. Za datoteku od `N` bajtova i `p` procesa, svaki proces čita `N / p` uzastopnih bajtova pozivom `MPI_File_read_shared`. Zajednički pokazivač se pomera za pročitani blok, a redosled zahteva određuje koji ulazni blok dobija koji proces.
2. Neka je `h = N / (2*p)` broj bajtova u jednoj polovini lokalnog bloka. Proces ranga `r` upisuje prvu polovinu od bajta `r*h`, a drugu od bajta `r*h + N/2`. Za četiri procesa izlaz je `A0 A1 A2 A3 B0 B1 B2 B3`, gde su `Ar` i `Br` polovine bloka koji je pročitao proces ranga `r`.
3. Tip `MPI_Type_vector(2, h, N/2, MPI_BYTE, ...)` opisuje dva izlazna bloka istog procesa. `MPI_File_set_view` pomera taj šablon za `r*h` bajtova. Svi procesi jednim kolektivnim pozivom `MPI_File_write_all` na `MPI_COMM_WORLD` upisuju svoje nepreklapajuće delove direktno iz lokalnih bafera.

Za 32 bajta i četiri procesa svaki čita osam bajtova, pa je `h = 4`. Ako P2 prvi dobije zajednički pokazivač, čita ulazne bajtove 0–7. U izlazu njegova prva polovina zauzima bajtove 8–11, a druga bajtove 24–27.

## Pokretanje

VS Code zadatak **Build and run MPI or OpenMP** pokreće program iz foldera izvornog fajla. Za njegovo pokretanje stavi `file1.dat` pored `mpi_file_rearrangement.c`. Tu će se pojaviti i `file2.dat`.

Iz ovog foldera kompajliraj program i pokreni ga sa četiri procesa. Za probu, sledeći primer pravi ulaz od 10.000.000 bajtova u privremenom folderu.

```sh
mpicc -std=c11 -Wall -Wextra mpi_file_rearrangement.c -o mpi_file_rearrangement
program="$PWD/mpi_file_rearrangement"
test_dir=$(mktemp -d)
(
    cd "$test_dir" || exit 1
    dd if=/dev/urandom of=file1.dat bs=1000000 count=10
    mpirun -np 4 "$program"
    wc -c file1.dat file2.dat
)
```

Obe datoteke treba da imaju 10.000.000 bajtova. Veličina ulaza mora biti pozitivna, deljiva sa `2*p` i najviše `INT_MAX` bajtova zbog broja elemenata u izvedenom tipu. Program upisuje rezultat preko postojećeg sadržaja `file2.dat`.

## Skica

![Zajedničko čitanje i kolektivni upis za četiri procesa](file_io_sketch.png)
