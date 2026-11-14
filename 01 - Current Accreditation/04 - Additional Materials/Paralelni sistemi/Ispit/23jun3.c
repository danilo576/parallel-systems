#include "mpi.h"
#include <stdio.h>

#define K 6
int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// a
	MPI_File fajl;
	MPI_File_open(MPI_COMM_WORLD, "puta.madre", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fajl);
	MPI_File_seek(fajl, rank * K * sizeof(int), MPI_SEEK_SET);

	int mat[K];
	for (int i = 0; i < K; i++)
	{
		int j = rank * K + i;
		mat[i] = j;
	}
	MPI_File_write(fajl, mat, K, MPI_INT, NULL);

	MPI_File_close(&fajl);

	// b
	MPI_File_open(MPI_COMM_WORLD, "puta.madre", MPI_MODE_RDONLY, MPI_INFO_NULL, &fajl);
	int mat1[K];
	MPI_File_read_at(fajl, rank * K * sizeof(int), mat1, K, MPI_INT, NULL);
	int gas = 0;
	for (int i = 0; i < K; i++)
	{
		if (mat[i] != mat1[i])
		{
			gas = 1;
			break;
		}
	}
	if (gas)
	{
		printf("DESI POSO NIJE DOBRO");
	}
	else
	{
		printf("DOBRO JE");
	}
	MPI_File_close(&fajl);

	// c
	MPI_File_open(MPI_COMM_WORLD, "madre.puta", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fajl);
	MPI_Datatype vetore;
	MPI_Type_vector(K / (int)(K / size), (int)(K / size), K, MPI_INT, &vetore);
	MPI_Type_commit(&vetore);
	MPI_File_set_view(fajl, rank * (int)(K / size) * sizeof(int), MPI_INT, vetore, "native", MPI_INFO_NULL);

	MPI_File_write_all(fajl, mat1, K, MPI_INT, NULL);

	MPI_File_close(&fajl);

	MPI_Finalize();
}
