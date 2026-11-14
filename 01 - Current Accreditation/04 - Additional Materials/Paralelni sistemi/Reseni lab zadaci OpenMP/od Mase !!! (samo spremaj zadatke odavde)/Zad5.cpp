#include <stdio.h>
#include <omp.h>
#include <iostream>

#define N 20000

void main5()
{
	int A[N], max, max_sec;
	double start, end;
	for (int i = 0; i < N; i++)
		A[i] = rand();

	start = omp_get_wtime();
	max_sec = -1;
	for (int i = 0; i < N; i++)
	{
		if (A[i] > max_sec)
			max_sec = A[i];
	}
	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);

	start = omp_get_wtime();
	max = -1;
	int private_max;
	#pragma omp parallel private(private_max)
	{
		private_max = -1;
		#pragma omp for
		for (int i = 0; i < N; i++)
		{
			if (A[i] > private_max)
				private_max = A[i];
		}
		#pragma omp critical
		if (private_max > max)
			max = private_max;
	}
	end = omp_get_wtime();
	printf("Parallel critical work took %f\n", end - start);
	if (max != max_sec)
		printf("Greska! %d != %d\n", max_sec, max);

	start = omp_get_wtime();
	max = -1;
	#pragma omp parallel for reduction(max: max)
	for (int i = 0; i < N; i++)
	{
		if (A[i] > max)
			max = A[i];
	}
	end = omp_get_wtime();
	printf("Parallel reduction work took %f\n", end - start);
	if (max != max_sec)
		printf("Greska! %d != %d\n", max_sec, max);
}