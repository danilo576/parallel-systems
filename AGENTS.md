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
- Za MPI zadatke sa logičkim uređenjem procesa koristi stil obojene skice iz `01 - Current Accreditation/02 - Practical Exercises/01 - MPI/exercises/logical_process_grid/exam_2020_07_03/matrix_vector_sketch.png`: prikaži stvarnu matricu sa obojenim vlasništvom elemenata, različitu i doslednu boju za svaki proces, logičku mrežu procesa i strelice relevantnih komunikacija. Raspodelu i indekse prilagodi konkretnom zadatku.

## Stil skica za MPI zadatke sa logičkom mrežom procesa

- Pre izrade pregledaj postojeće skice i njihove promptove u `exercises/logical_process_grid`, posebno `exam_2020_07_03` i `exam_2022_06_23`. Koristi ih kao vizuelne reference, a raspodelu i komunikacije izvodi iz konkretnog zadatka i rešenja.
- Skicu generiši ugrađenim `imagegen` alatom, uz postojeću skicu kao stilsku referencu. Prati izgled nastavnog lista: svetla papirna pozadina, pastelna obojena polja i naslovi, jasno odvojene celine, čitljivi indeksi i strelice. Nemoj je zameniti običnom programatski nacrtanom tabelom ili mrežom pravougaonika.
- Prikaži stvarne matrice i vektore sa globalnim indeksima, vlasništvo svakog elementa i odvojenu logičku mrežu procesa. Svakom procesu dodeli posebnu boju i dosledno je koristi kroz sve prikaze. Za vektor jasno naznači koje grupe indeksa pripadaju istoj koloni ili vrsti mreže.
- Odvojeno prikaži raspodelu podataka, lokalno računanje i sklapanje rezultata. Uz komunikacione strelice navedi grupnu operaciju, komunikator i lokalni rang korena; smer strelica i redosled faza moraju odgovarati kodu.
- Dodaj konkretan primer lokalnog bloka i računanja jednog procesa, kao i primer sabiranja doprinosa. Prikaži odredišni proces i konačan rezultat kada ih zadatak zahteva. Kod ciklične raspodele posebno označi globalne indekse pri sklapanju rezultata.
- Sačuvaj korišćeni prompt uz sliku kao `*_sketch_prompt.txt`, prikaži sliku u README-u i vizuelno proveri generisanu sliku: oznake procesa, indekse, boje, brojeve, korene, strelice i čitljivost. Ispravi uočene greške pre završetka.

## Dopunske GitHub reference

Koristi ove izvore za dodatne primere pri rešavanju zadataka. Prednost daj materijalima iz projektnog foldera.

- [VIII-Semestar / Paralelni Sistemi](https://github.com/sssteeefaaan/VIII-Semestar/tree/main/Paralelni%20Sistemi)
- [PS-OpenMP](https://github.com/andrijacenic/PS-OpenMP)
- [PS-MPI](https://github.com/andrijacenic/PS-MPI)
