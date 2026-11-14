#include <stdio.h>
#include <omp.h>

#define N 5000

void main3()
{
	int rez, rez_sec;
	double start, end;

	start = omp_get_wtime();
	rez_sec = 0;
	for (int i = 2; i < N; i++)
	{
		int prime = 1;
		int m = i / 2;
		for (int j = 2; j <= m; j++)
		{
			if (i % j == 0)
			{
				prime = 0;
				break;
			}
		}
		rez_sec += prime;
	}
	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);

	start = omp_get_wtime();
	rez = 0;
	int private_rez;
	#pragma omp parallel private(private_rez)
	{
		private_rez = 0;
		int prime, m, j;
		#pragma omp for private(prime, m, j)
		for (int i = 2; i < N; i++)
		{
			prime = 1;
			m = i / 2;
			for (j = 2; j <= m; j++)
			{
				if (i % j == 0)
				{
					prime = 0;
					break;
				}
			}
			private_rez += prime;
		}
		#pragma omp critical
		rez += private_rez;
	}
	end = omp_get_wtime();
	printf("Parallel outer critical took %f\n", end - start);
	if (rez != rez_sec)
		printf("Greska! %d != %d\n", rez, rez_sec);

	start = omp_get_wtime();
	rez = 0;
	#pragma omp parallel for reduction(+: rez)
	for (int i = 2; i < N; i++)
	{
		int prime = 1;
		for (int j = 2; j <= i / 2; j++)
		{
			if (i % j == 0)
			{
				prime = 0;
				break;
			}
		}
		rez += prime;
	}
	end = omp_get_wtime();
	printf("Parallel outer reduction took %f\n", end - start);
	if (rez != rez_sec)
		printf("Greska! %d != %d\n", rez, rez_sec);
}