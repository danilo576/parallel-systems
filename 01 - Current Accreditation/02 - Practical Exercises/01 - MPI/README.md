# MPI vežbe

## Zadaci

| Folder | Sadržaj |
| --- | --- |
| [exercises/hello](exercises/hello) | Osnovni MPI program |
| [exercises/chain](exercises/chain) | Lančana komunikacija |
| [exercises/sum](exercises/sum) | Sabiranje |
| [exercises/tree_sum](exercises/tree_sum) | Sabiranje u strukturi stabla |
| [exercises/matrix_product](exercises/matrix_product) | Množenje sa delimičnim doprinosima rezultatu |
| [exercises/cyclic_matrix_product](exercises/cyclic_matrix_product) | Ciklična raspodela kolona, kod i skica |
| [exercises/column_matrix_product](exercises/column_matrix_product) | Blokovska raspodela kolona, kod i skica |

## Zajednički fajlovi

- `common/mpi_helpers.h` - helperi za ispis matrica i raspodele.
- `materials/` - prezentacije i pregled MPI funkcija (lokalni PDF fajlovi).
- `bin/` - ranije kompajlirani programi, sačuvani lokalno.

Za uključivanje helpera iz foldera zadatka koristi:

```c
#include "../../common/mpi_helpers.h"
```

Otvori izvorni `.c` fajl i pokreni postojeći VS Code zadatak **Build and run MPI or OpenMP**. On radi i iz podfoldera; novi izvršni fajl nastaje pored izvornog koda.
