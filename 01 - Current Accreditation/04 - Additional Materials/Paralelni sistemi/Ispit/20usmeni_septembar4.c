#include "omp.h"
#include <stdio.h>

#define N 10
#define niti 4
int main(int argc, char const *argv[])
{
	int a[N];
	for (int i = 1; i < N + 1; i++)
	{
		a[i - 1] = i;
	}
	int p = 1;
	omp_set_num_threads(niti);
#pragma omp parallel
	{
		int q = 1;
#pragma omp for
		for (int i = 0; i < N; i++)
		{
			q *= a[i];
		}

		printf("%d \n", q);
#pragma omp critical
		p *= q;
	}
	printf("%d \n", p);
	return 0;
}
