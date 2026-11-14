#include "mpi.h"
#include <stdio.h>

#define N 4 // deljivo sa size
int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	typedef struct struct_radnik
	{
		int id;
		int plata;
		char ime[30];
	} radnik;

	radnik radnici[N];

	MPI_Aint adrese[3];
	MPI_Get_address(&radnici[0].id, &adrese[0]);
	MPI_Get_address(&radnici[0].plata, &adrese[1]);
	MPI_Get_address(&radnici[0].ime, &adrese[2]);

	adrese[2] = adrese[2] - adrese[0];
	adrese[1] = adrese[1] - adrese[0];
	adrese[0] = 0;

	MPI_Datatype radnikTip;
	MPI_Type_create_struct(3, (int[]){1, 1, 30}, adrese, (MPI_Datatype[]){MPI_INT, MPI_INT, MPI_CHAR}, &radnikTip);
	MPI_Type_commit(&radnikTip);

	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			radnici[i].id = i;
			scanf("%d", &(radnici[i].plata));
			scanf("%s", radnici[i].ime);
		}
	}

	radnik radniciNovi[N];
	MPI_Scatter(radnici, N / size, radnikTip, radniciNovi, N / size, radnikTip, 0, MPI_COMM_WORLD);

	typedef struct struct_reduce
	{
		int plata;
		int id;
	} reducir;
	reducir r;

	r.id = radniciNovi[0].id;
	r.plata = radniciNovi[0].plata;
	for (int i = 1; i < N / size; i++)
	{
		if (radniciNovi[i].plata < r.plata)
		{
			r.id = radniciNovi[i].id;
			r.plata = radniciNovi[i].plata;
		}
	}
	reducir q;
	MPI_Reduce(&r, &q, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		printf("%d %d %d; ", rank, q.plata, q.id);
	}

	MPI_Finalize();
}
