#include <stdio.h>
#include <omp.h>

#define N 20000

void main4()
{
	int A[N];
	double avg, avg_sec;
	double start, end;
	for (int i = 0; i < N; i++)
		A[i] = i + 3;

	start = omp_get_wtime();
	avg_sec = 0;
	for (int i = 0; i < N; i++)
		avg_sec += A[i] / (double)N;
	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);

	start = omp_get_wtime();
	avg = 0;
	double private_avg;
	#pragma omp parallel private(private_avg)
	{
		private_avg = 0;
		#pragma omp for
		for (int i = 0; i < N; i++)
			private_avg += A[i] / (double)N;
		#pragma omp critical
			avg += private_avg;
	}
	end = omp_get_wtime();
	printf("Parallel critical work took %f\n", end - start);
	if (avg != avg_sec)
		printf("Greska! %f != %f\n", avg, avg_sec);

	start = omp_get_wtime();
	avg = 0;
	#pragma omp parallel for reduction(+: avg)
	for (int i = 0; i < N; i++)
		avg += A[i] / (double)N;
	end = omp_get_wtime();
	printf("Parallel reduction work took %f\n", end - start);
	if (avg != avg_sec)
		printf("Greska! %f != %f\n", avg, avg_sec);
}