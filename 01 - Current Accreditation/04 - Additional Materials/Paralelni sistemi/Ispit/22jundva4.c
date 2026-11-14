#include "omp.h"
#include <stdio.h>

#define N 40
#define niti 4
int main(int argc, char const *argv[])
{
	int g = 10;
	int x[N];
	int y[N];
	int z[N];
	for (int i = 0; i < N; i++)
	{
		x[i] = y[i] = z[i] = i;
	}
	omp_set_num_threads(niti);
	g = g + z[0];
	x[1] = x[1] + z[1];
#pragma omp parallel for reduction(+ : g)
	for (int i = 2; i < N; i++)
	{
		y[i - 1] = y[i - 1] + x[i];
		g = g + z[i - 1];
		x[i] = x[i] + z[i];
	}
	y[N - 1] = y[N - 1] + x[N];

	printf("%d \n", g);
	for (int i = 0; i < N; i++)
	{
		printf("%d \n", x[i]);
	}
	return 0;
}
