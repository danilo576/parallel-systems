#include <stdio.h>

#define N 5
int main()
{
	int b[N];
	int a[N];
	int d = 2;
	for (int i = 0; i < N; i++)
	{
		a[i] = b[i] = i;
	}
#pragma omp parallel num_threads(4)
	{
#pragma omp for reduction(* : d)
		for (int i = 1; i < N; i++)
		{
			d = b[i];
			a[i] = a[0];
			for (int j = 1; j <= i; j++)
			{
				a[i] += b[j];
			}
		}
	}
	printf("%d\n", d);
	for (int i = 0; i < N; i++)
	{
		printf("%d\n", a[i]);
	}
}