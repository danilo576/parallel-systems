# Paralelni sistemi

Kod, lične beleške i rešenja zadataka iz predmeta Paralelni sistemi.
Literatura, prezentacije i kompajlirani programi ostaju lokalno i nisu deo repozitorijuma.

## Organizacija

- `01 - Current Accreditation/` — materijali i rad prema aktuelnoj akreditaciji.
- `02 - Previous Accreditation/` — rad vezan za prethodnu akreditaciju.
- `.vscode/` — podešavanja kompajliranja i pokretanja.
- `AGENTS.md` — smernice za rad sa AI asistentom.

Git čuva fajlove, pa folderi koji sadrže samo ignorisanu literaturu neće biti vidljivi na GitHub-u.
Beleške piši kao Markdown (`.md`) uz odgovarajući zadatak ili u podfolderu `notes/`.

## Pokretanje primera

Potrebni su MPI implementacija sa alatima `mpicc` i `mpirun`, kao i C kompajler koji podržava OpenMP.
Postojeća VS Code podešavanja prilagođena su macOS-u na Apple Silicon računaru:
Open MPI u `/opt/homebrew`, GCC 16 i Command Line Tools SDK.
Na drugom računaru prilagodi putanje i verziju kompajlera u `.vscode/`.

U terminalu, iz korena repozitorijuma:

```sh
mkdir -p build
mpicc "01 - Current Accreditation/02 - Practical Exercises/01 - MPI/hello_mpi.c" -o build/hello_mpi
mpirun -np 4 ./build/hello_mpi

gcc-16 -fopenmp "01 - Current Accreditation/02 - Practical Exercises/02 - OpenMP/hello_openmp.c" -o build/hello_openmp
OMP_NUM_THREADS=4 ./build/hello_openmp
```

U VS Code-u otvori C fajl i pokreni podrazumevani build task (`Cmd+Shift+B`).
Task traži broj MPI procesa ili OpenMP niti. Redosled ispisa procesa i niti može da se razlikuje između pokretanja.

## Održavanje

1. Pre rada na drugom računaru preuzmi promene: `git pull --ff-only`.
2. Reši jednu smislenu celinu i proveri da se kod kompajlira i radi.
3. Pregledaj izmene pomoću `git status` i `git diff`.
4. Dodaj konkretne fajlove: `git add "putanja/do/fajla"`.
5. Pregledaj pripremljene izmene: `git diff --staged`.
6. Sačuvaj ih opisnim commit-om, na primer `git commit -m "Dodato MPI resenje za razmenu poruka"`.
7. Pošalji promene: `git push`.

Za veće eksperimente koristi posebnu granu, na primer `git switch -c mpi-kolektivne-operacije`.
Za samostalan rad na manjim zadacima dovoljna je grana `main`.

`.gitignore` dozvoljava izvorni kod, Markdown i tekstualne beleške, skripte i navedena podešavanja.
Kada uvodiš novi format, prvo proveri pravila ignorisanja. Ako fajl nedostaje u `git status`,
koristi `git check-ignore -v "putanja/do/fajla"`.
Ne dodaj lozinke, tokene ili privatne podatke u kod i beleške.
