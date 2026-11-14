#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int boja = ((int)(rank / (int)(sqrt(size)))) == (rank % (int)(sqrt(size)));
	MPI_Comm novakDjokovic;
	MPI_Comm_split(MPI_COMM_WORLD, boja, 0, &novakDjokovic);
	int rankNovak;
	MPI_Comm_rank(novakDjokovic, &rankNovak);
	char gas[3];
	if (rankNovak == 0)
	{
		gas[0] = 'G';
		gas[1] = 'a';
		gas[2] = 's';
	}
	MPI_Bcast(gas, 3, MPI_CHAR, 0, novakDjokovic);
	if (boja)
		printf("%s", gas);

	MPI_Finalize();
}
