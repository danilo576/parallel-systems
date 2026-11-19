# MPI vežbe

## Zadaci

| Folder | Sadržaj |
| --- | --- |
| [exercises/basic_communication/hello](exercises/basic_communication/hello) | Osnovni MPI program |
| [exercises/basic_communication/chain](exercises/basic_communication/chain) | Lančana komunikacija |
| [exercises/basic_communication/sum](exercises/basic_communication/sum) | Sabiranje |
| [15.05.2015, I kolokvijum, zadatak 3](exercises/basic_communication/colloquium_2015_05_15) | Slanje jednog podatka od P0 kroz stablo |
| [exercises/basic_communication/tree_sum](exercises/basic_communication/tree_sum) | Sabiranje u strukturi stabla — 04.07.2015, zadatak 4(a). |
| [exercises/basic_communication/exam_2019_06_12](exercises/basic_communication/exam_2019_06_12) | Prosleđivanje niza zaposlenih — 12.06.2019, zadatak 4(a). |
| [exercises/matrix_matrix_products/matrix_product](exercises/matrix_matrix_products/matrix_product) | Množenje sa delimičnim doprinosima rezultatu |
| [exercises/matrix_matrix_products/cyclic_matrix_product](exercises/matrix_matrix_products/cyclic_matrix_product) | Ciklična raspodela kolona, kod i skica |
| [exercises/matrix_matrix_products/column_matrix_product](exercises/matrix_matrix_products/column_matrix_product) | Blokovska raspodela kolona, kod i skica |
| [exercises/matrix_vector_products/column_matrix_vector_product](exercises/matrix_vector_products/column_matrix_vector_product) | Proizvod matrice i vektora, uzastopne kolone i grupne operacije |
| [exercises/matrix_vector_products/column_matrix_vector_min](exercises/matrix_vector_products/column_matrix_vector_min) | Samostalan rad: proizvod matrice i vektora i minimum svake vrste |
| [exercises/logical_process_grid](exercises/logical_process_grid) | Zadaci sa logičkom mrežom procesa — 12.06.2019, 03.07.2020, 24.04.2021, 07.10.2021, 23.06.2022. i 02.10.2023. |
| [25.09.2025, zadatak 1a](exercises/matrix_matrix_products/exam_2025_09_25) | Množenje matrica i minimumi vrsta A |
| [25.09.2025, zadatak 1b](exercises/logical_process_grid/exam_2025_09_25) | Komunikator na sporednoj dijagonali i najveći proizvod vrste |
| [exercises/virtual_topologies/exam_2020_07_03](exercises/virtual_topologies/exam_2020_07_03) | Virtuelne topologije, ciklični pomak po vrstama — 03.07.2020, zadatak 4(a). |
| [exercises/virtual_topologies/exam_2021_04_24](exercises/virtual_topologies/exam_2021_04_24) | Virtuelne topologije, ciklični pomak ulevo po vrstama — 24.04.2021, zadatak 4(a). |
| [exercises/virtual_topologies/exam_2023_04_25](exercises/virtual_topologies/exam_2023_04_25) | Virtuelne topologije, razmena unutar vrste između kolona iste parnosti — 25.04.2023. |
| [exercises/parallel_io/exam_2021_07_02](exercises/parallel_io/exam_2021_07_02) | MPI-IO, obrnuti upis i blokovi rastuće dužine — 02.07.2021, zadatak 3. |
| [exercises/parallel_io/exam_2021_10_07](exercises/parallel_io/exam_2021_10_07) | MPI-IO, upis blokova rastuće dužine uz računanje — 07.10.2021, zadatak 3. |
| [exercises/parallel_io/exam_2022_01_28](exercises/parallel_io/exam_2022_01_28) | MPI-IO, zajedničko čitanje i kolektivni upis — 28.01.2022. |
| [exercises/parallel_io/exam_2022_12_05](exercises/parallel_io/exam_2022_12_05) | MPI-IO, upis blokova rastuće dužine — 05.12.2022. |
| [exercises/parallel_io/exam_2023_09_22](exercises/parallel_io/exam_2023_09_22) | MPI-IO, množenje matrica po blokovima vrsta i izbor odredišnog procesa po najmanjoj sumi — 22.09.2023, zadatak 1. |

## Zajednički fajlovi

- `common/mpi_helpers.h` - helperi za ispis matrica i raspodele.
- `materials/` - prezentacije i pregled MPI funkcija (lokalni PDF fajlovi).
- `bin/` - ranije kompajlirani programi, sačuvani lokalno.

Za uključivanje helpera iz foldera zadatka koristi:

```c
#include "../../../common/mpi_helpers.h"
```

Otvori izvorni `.c` fajl i pokreni postojeći VS Code zadatak **Build and run MPI or OpenMP**. On radi i iz podfoldera; novi izvršni fajl nastaje pored izvornog koda.
