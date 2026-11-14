#include "omp.h"
#include <stdio.h>

#define N 4
#define niti 4
int main(int argc, char const *argv[])
{
	int j = N + 1;
	int x = 0;
	int a[2 * N + 1];
	int b[2 * N + 1];
	for (int i = 0; i < 2 * N + 1; i++)
	{
		a[i] = b[i] = i;
	}
#pragma omp parallel
	{
		omp_set_num_threads(niti);
#pragma omp for reduction(+ : x)
		for (int i = 0; i < N; i++)
		{
			x += a[i];
			b[i] = b[i] + b[N + 1 + i];
		}
	}
	printf("%d \n", x);
	for (int i = 0; i < 2 * N + 1; i++)
	{
		printf("%d \n", b[i]);
	}
	return 0;
}
