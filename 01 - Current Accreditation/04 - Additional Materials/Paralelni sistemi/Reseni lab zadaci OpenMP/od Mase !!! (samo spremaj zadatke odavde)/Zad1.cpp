#include <stdio.h>
#include <omp.h>

#define N 200

void main1()
{
	double start, end;
	int A[N][N], B[N][N], C[N][N], C_sek[N][N];
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			A[i][j] = i + j;
			B[i][j] = i * j;
		}
	}

	int proc = omp_get_num_procs();
	omp_set_num_threads(proc);

	start = omp_get_wtime();
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			C_sek[i][j] = 0;
			for (int k = 0; k < N; k++)
			{
				C_sek[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	end = omp_get_wtime();
	printf("Sequential work took %f\n", end - start);
	
	int i, j, k;

	//shit jer svi pristupaju istom elementu za azuriranje, vrlo neoptimalno (sporije od sekvencijalnog)
	start = omp_get_wtime();
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			int p = 0;
			#pragma omp parallel for reduction(+: p)
				for (k = 0; k < N; k++)
				{
					p += A[i][k] * B[k][j];
				}
			C[i][j] = p;
		}
	}
	end = omp_get_wtime();
	printf("Parallel reduction work took %f\n", end - start);
	for (i = 0; i < N; i++)
	{
		bool wrong = false;
		for (j = 0; j < N; j++)
		{
			if (C[i][j] != C_sek[i][j])
			{
				printf("Greska! (%d, %d)\n", i, j);
				wrong = true;
				break;
			}
		}
		if (wrong)
			break;
	}

	//Okej je, za veliku kolicinu posla brze od sekvencijalnog. Paralelizuje se izracunavanje elemenata jedne vrste C
	start = omp_get_wtime();
	for (i = 0; i < N; i++)
	{
		int k;
		#pragma omp parallel for private(k)
			for (j = 0; j < N; j++)
			{
				C[i][j] = 0;
				for (k = 0; k < N; k++)
				{
					C[i][j] += A[i][k] * B[k][j];
				}
			}
	}
	end = omp_get_wtime();
	printf("Parallel j work took %f\n", end - start);
	for (i = 0; i < N; i++)
	{
		bool wrong = false;
		for (j = 0; j < N; j++)
		{
			if (C[i][j] != C_sek[i][j])
			{
				printf("Greska! (%d, %d)\n", i, j);
				wrong = true;
				break;
			}
		}
		if (wrong)
			break;
	}

	//Najbolje resenje jer se paralelizuje najveca kolicina posla. Paralelno se racunaju vrste matrice C. Vec za N = 20 radi bolje od sekvencijalnog
	start = omp_get_wtime();
	#pragma omp parallel for
		for (i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				C[i][j] = 0;
				for (int k = 0; k < N; k++)
				{
					C[i][j] += A[i][k] * B[k][j];
				}
			}
		}
	end = omp_get_wtime(); 
	printf("Parallel i work took %f\n", end - start);
	for (i = 0; i < N; i++)
	{
		bool wrong = false;
		for (j = 0; j < N; j++)
		{
			if (C[i][j] != C_sek[i][j])
			{
				printf("Greska! (%d, %d)\n", i, j);
				wrong = true;
				break;
			}
		}
		if (wrong)
			break;
	}

	//Najbrza je optimizacija i po i i po j
	start = omp_get_wtime();
	#pragma omp parallel for
	for (i = 0; i < N; i++)
	{
		#pragma omp parallel for private(k)
		for (j = 0; j < N; j++)
		{
			C[i][j] = 0;
			for (k = 0; k < N; k++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	end = omp_get_wtime();
	printf("Parallel i, j work took %f\n", end - start);
	for (i = 0; i < N; i++)
	{
		bool wrong = false;
		for (j = 0; j < N; j++)
		{
			if (C[i][j] != C_sek[i][j])
			{
				printf("Greska! (%d, %d)\n", i, j);
				wrong = true;
				break;
			}
		}
		if (wrong)
			break;
	}

	//Reduction po k usporava posao, bolje je bez njega
	start = omp_get_wtime();
	#pragma omp parallel for
	for (i = 0; i < N; i++)
	{
		#pragma omp parallel for
		for (j = 0; j < N; j++)
		{
			int p = 0;
			#pragma omp parallel for reduction(+: p)
			for (k = 0; k < N; k++)
			{
				p += A[i][k] * B[k][j];
			}
			C[i][j] = p;
		}
	}
	end = omp_get_wtime();
	printf("Parallel i, j, k work took %f\n", end - start);
	for (i = 0; i < N; i++)
	{
		bool wrong = false;
		for (j = 0; j < N; j++)
		{
			if (C[i][j] != C_sek[i][j])
			{
				printf("Greska! (%d, %d)\n", i, j);
				wrong = true;
				break;
			}
		}
		if (wrong)
			break;
	}
}