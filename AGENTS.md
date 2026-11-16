# Paralelni sistemi

- Za sve zadatke iz Paralelnih sistema kao primarne izvore koristi literaturu, prezentacije i primere iz projektnog foldera `02 - Parallel Systems`.
- Među lokalnim izvorima proveri i `01 - Current Accreditation/04 - Additional Materials/Paralelni sistemi` za relevantne ispitne zadatke, rešene vežbe i skripte.
- Pri rešavanju zadataka prati sintaksu, metode i način rešavanja iz dostupnih materijala.
- Objašnjenja piši na srpskom jeziku, latinicom. Postupak prikaži jasno, korak po korak.
- Tekst zadatka navedi na srpskom jeziku, latinicom, u komentaru iznad funkcije `main`.
- Kod (nazive promenljivih i funkcija), poruke programa i sve ostale komentare piši na engleskom jeziku.
- Imena izvornih fajlova piši na engleskom jeziku, uz odgovarajuću ekstenziju (npr. `mpi_chain.c`).
- Biraj što jednostavnije rešenje koje ispunjava zadatak. Ne dodaj nepotrebnu sinhronizaciju, dodatne potvrde prijema ili složeno formatiranje izlaza.

- Ne koristi `MPI_Type_free` u rešenjima zadataka.
- Ne koristi `MPI_Comm_free` u rešenjima zadataka.
- Ne koristi `MPI_Allreduce` u rešenjima zadataka. Kada rezultat redukcije treba svim procesima, koristi `MPI_Reduce` i `MPI_Bcast`.
- Za svaki MPI zadatak generiši sliku kao skicu raspodele podataka po procesima i relevantne komunikacije. Sačuvaj sliku uz zadatak i prikaži je u njegovom `README.md` fajlu.
- Za MPI zadatke sa logičkim uređenjem procesa koristi stil obojene skice iz `01 - Current Accreditation/02 - Practical Exercises/01 - MPI/exercises/exam_2020_07_03/matrix_vector_sketch.png`: prikaži stvarnu matricu sa obojenim vlasništvom elemenata, različitu i doslednu boju za svaki proces, logičku mrežu procesa i strelice relevantnih komunikacija. Raspodelu i indekse prilagodi konkretnom zadatku.

## Dopunske GitHub reference

Koristi ove izvore za dodatne primere pri rešavanju zadataka. Prednost daj materijalima iz projektnog foldera.

- [VIII-Semestar / Paralelni Sistemi](https://github.com/sssteeefaaan/VIII-Semestar/tree/main/Paralelni%20Sistemi)
- [PS-OpenMP](https://github.com/andrijacenic/PS-OpenMP)
- [PS-MPI](https://github.com/andrijacenic/PS-MPI)
