#include "mpi.h"
#include <stdio.h>
#include <math.h>
// Udari skip na ispit ako bude, muka mi bilo dok sam pisao ovo
#define N 8
int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Datatype blokMatrice;

	MPI_Type_vector(N * N / size, 1, (int)(sqrt(size)), MPI_INT, &blokMatrice);
	MPI_Type_commit(&blokMatrice);

	int mat[N][N];
	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				mat[i][j] = i * N + j;
			}
		}
		int y = 0;
		for (int i = 0; i < N; i += N / sqrt(size))
			for (int j = 0; j < sqrt(size); j++)
			{
				MPI_Send(&mat[i][j], 1, blokMatrice, y++, 0, MPI_COMM_WORLD);
				printf("%d %d \n", i, j);
			}
	}
	int mat_loc[(int)(N / sqrt(size))][(int)(N / sqrt(size))];
	MPI_Recv(mat_loc, N * N / size, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			printf("%d %d %d %d\n", rank, i, j, mat_loc[i][j]);
		}
	}

	int vrsta = rank / (int)(sqrt(size));
	int kolona = rank % (int)(sqrt(size));
	MPI_Comm kolonka;
	MPI_Comm_split(MPI_COMM_WORLD, kolona, 0, &kolonka);
	int kolonaRank;
	MPI_Comm_rank(kolonka, &kolonaRank);
	MPI_Comm vrstica;
	MPI_Comm_split(MPI_COMM_WORLD, vrsta, 0, &vrstica);
	int vrstaRank;
	MPI_Comm_rank(vrstica, &vrstaRank);

	MPI_Datatype blokVektor, blokVektorUsrani;

	MPI_Type_vector(N / (int)(sqrt(size)), 1, (int)(sqrt(size)), MPI_INT, &blokVektor);
	MPI_Type_create_resized(blokVektor, 0, 1 * sizeof(int), &blokVektorUsrani);
	MPI_Type_commit(&blokVektorUsrani);

	int vektor[N];
	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			vektor[i] = i;
		}
	}
	int vektorNov[N / (int)(sqrt(size))];
	if (kolonaRank == 0)
		MPI_Scatter(vektor, 1, blokVektorUsrani, vektorNov, N / (int)(sqrt(size)), MPI_INT, 0, vrstica);
	MPI_Bcast(vektorNov, N / (int)(sqrt(size)), MPI_INT, 0, kolonka);
	int rez[N / (int)(sqrt(size))];

	for (int i = 0; i < N / (int)(sqrt(size)); i++)
	{
		rez[i] = 0;
		for (int j = 0; j < N / (int)(sqrt(size)); j++)
		{
			rez[i] += mat_loc[i][j] * vektorNov[j];
		}
	}
	int rezrez[N / (int)(sqrt(size))];
	MPI_Reduce(rez, rezrez, N / (int)(sqrt(size)), MPI_INT, MPI_SUM, 0, vrstica);

	int vektorGedr[N];

	MPI_Gather(rezrez, N / (int)(sqrt(size)), MPI_INT, vektorGedr, N / (int)(sqrt(size)), MPI_INT, 0, kolonka);

	printf("%d ka\n", rank);
	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			printf("%d \n", vektorGedr[i]);
		}
	}
	MPI_Finalize();
}
