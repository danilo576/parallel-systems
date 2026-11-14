#include "omp.h"
#include <stdio.h>
#include <stdlib.h>

#define N 12
#define niti 4
int main(int argc, char const *argv[])
{
	int a[N];
	for (int i = 0; i < N; i++)
	{
		a[i] = rand() % 100;
		printf("%d \n", a[i]);
	}
	printf("\n");
	int max = 0;
// sa critical
// #pragma omp parallel
// 	{
// 		omp_set_num_threads(niti);
// #pragma omp for
// 		for (int i = 0; i < N; i++)
// 		{
// #pragma omp critical
// 			if (max < a[i])
// 				max = a[i];
// 		}
// 	}
// bez critical
#pragma omp parallel
	{
		omp_set_num_threads(niti);
#pragma omp for reduction(max : max)
		for (int i = 0; i < N; i++)
		{
			if (max < a[i])
				max = a[i];
		}
	}
	printf("%d \n", max);
	return 0;
}
