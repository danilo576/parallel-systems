#include "omp.h"
#include <stdio.h>

#define N 4
#define M 4
#define niti 4
int main(int argc, char const *argv[])
{
	int x = 0;
	int a[N][M];
	int b[N][M];
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			a[i][j] = b[i][j] = i * M + j;
		}
	}
#pragma omp parallel
	{
		omp_set_num_threads(niti);
#pragma omp for reduction(+ : x)
		for (int j = 0; j < M; j++)
		{
			for (int i = 2; i < N; i++)
			{
				x = x + a[i][j];
				b[i][j] = 4 * b[i - 2][j];
			}
		}
	}
	printf("%d \n", x);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			printf("%d \n", b[i][j]);
		}
	}
	return 0;
}
