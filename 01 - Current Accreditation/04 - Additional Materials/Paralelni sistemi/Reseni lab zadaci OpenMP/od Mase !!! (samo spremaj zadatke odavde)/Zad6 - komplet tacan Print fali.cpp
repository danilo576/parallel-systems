#include <stdio.h>
#include <omp.h>
#include <iostream>

#define n 500
#define m 500

void main()
{
	int a[m][n];
	double start, end;
	start = omp_get_wtime();
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			a[i][j] = i + j;

	for (int i = 1; i < m; i++)
		for (int j = 0; j < n; j++)
			a[i][j] = 2 * a[i - 1][j];

	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);

	start = omp_get_wtime();
	for (int i = 1; i < m; i++)
#pragma omp parallel for
		for (int j = 0; j < n; j++)
			a[i][j] = 2 * a[i - 1][j];

	end = omp_get_wtime();
	printf("Private j work took %f\n", end - start);


	start = omp_get_wtime();
	int j, i;
#pragma omp parallel for private(i)
	for (j = 0; j < n; j++)
		for (i = 1; i < m; i++)
			a[i][j] = 2 * a[i - 1][j];

	end = omp_get_wtime();
	printf("Zamenjena mesta work took %f\n", end - start);
}