#include <stdio.h>
#include <omp.h>

#define N 10000

void main2()
{
	int A[N], B[N], rez, rez_sec;
	double start, end;
	for (int i = 0; i < N; i++)
	{
		A[i] = i + 2;
		B[i] = i + 4;
	}

	rez_sec = 0;
	start = omp_get_wtime();
	for (int i = 0; i < N; i++)
		rez_sec += A[i] * B[i];
	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);

	rez = 0;
	int private_rez;
	start = omp_get_wtime();
	#pragma omp parallel private(private_rez)
	{
		private_rez = 0;
		#pragma omp for
		for (int i = 0; i < N; i++)
			private_rez += A[i] * B[i];
		#pragma omp critical
		rez += private_rez;
	}
	end = omp_get_wtime();
	printf("Parallel critical work took %f\n", end - start);
	if (rez != rez_sec)
		printf("Greska! %d != %d\n", rez_sec, rez);

	rez = 0;
	start = omp_get_wtime();
	#pragma omp parallel for reduction(+: rez)
	for (int i = 0; i < N; i++)
		rez += A[i] * B[i];
	end = omp_get_wtime();
	printf("Parallel reduction work took %f\n", end - start);
	if (rez != rez_sec)
		printf("Greska! %d != %d\n", rez_sec, rez);
}