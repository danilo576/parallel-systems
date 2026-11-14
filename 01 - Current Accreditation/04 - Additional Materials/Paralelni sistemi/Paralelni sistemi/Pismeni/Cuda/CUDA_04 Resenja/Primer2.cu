#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <device_launch_parameters.h>
#include <cuda.h>
#include <cuda_runtime_api.h>

#define N 20
#define br_niti 8

void HANDLE_ERROR(cudaError_t call)
{
    cudaError_t ret = call;
    switch(ret)
    {
       case cudaSuccess:
	break;
       case cudaErrorInvalidValue:
	printf("ERROR: InvalidValue:%i.\n",__LINE__);
         exit(-1);
         break;
       case cudaErrorInvalidMemcpyDirection:
	printf("ERROR:Invalid memcpy direction:%i.\n",__LINE__);
         exit(-1);
         break;
       default:
	printf("ERROR>line:%i.%d' ‘ %s\n",__LINE__,ret,cudaGetErrorString(ret));
         exit(-1);
         break;   
      }
}

__global__ void kernel(int* niz, int* min, int* max)
{

	int tid = threadIdx.x; 
	__shared__ int sh_min[br_niti];
	__shared__ int sh_max[br_niti];
	
	//Primer: niz: 0 1 2 3 4 5 6 7 8 9 10 11 12, 3 niti, nit 0 obrađuje elemente 0,1,2,3; nit 1 - 4 5 6 7; nit 2 - 8,9,10,11,12;
	
	int slice = N / br_niti; //Koliko elemenata obradjuje svaka nit
	
	int start = tid * slice;
	
	if (tid == br_niti - 1) //Poslednja nit u bloku
	{
		slice += N % br_niti;
	}
	
	int end = start + slice;
	int local_min = niz[start];
	int local_max = niz[start];
	
	for (int i = start; i < end; i++)
	{
		if (niz[i] > local_max)
			local_max = niz[i];
		if (niz[i] < local_min)
			local_min = niz[i];
	}
	
	//Princip redukcije! Referenca: Chapter 5.3, Jason Sanders, Edward Kandrot: CUDA by example: an introduction to general-purpose GPU programming, Addison-Wesley Professional

	sh_max[tid] = local_max;
	sh_min[tid] = local_min;
	
	__syncthreads();
	
	int half = br_niti;
	do{
		__syncthreads();
		half /= 2;
		if (tid < half)
		{
			if (sh_min[tid] > sh_min[tid + half])
				sh_min[tid] = sh_min[tid + half];
		
			if (sh_max[tid] < sh_max[tid + half])
				sh_max[tid] = sh_max[tid + half];
		}
	} while(half!=1);
	
	if (tid == 0)
	{
		*min = sh_min[0];
		*max = sh_max[0];
	}
}

int main(){

	int *h_A, *d_A, *d_min, *d_max, h_min, h_max;
	
	//U slučaju da zadatak radi sa matricom, preporuka je deklarisati matricu kao niz sa NzN elemenata (u slučaju da je matrica NxN).
	
	h_A = (int*)malloc(N * sizeof(int));
	
	for (int i =0; i< N; i++)
		h_A[i] = rand() % 100;

	printf("Niz A:\n");
	for (int i =0; i< N; i++)	
		printf("%d ", h_A[i]);

	HANDLE_ERROR(cudaMalloc((void**)&d_A, N * sizeof(int)));
	
	HANDLE_ERROR(cudaMalloc((void**)&d_min, sizeof(int)));
	
	HANDLE_ERROR(cudaMalloc((void**)&d_max, sizeof(int)));

	HANDLE_ERROR(cudaMemcpy(d_A, h_A, N * sizeof(int), cudaMemcpyHostToDevice));

	kernel<<<1, br_niti>>>(d_A, d_min, d_max);

	HANDLE_ERROR(cudaMemcpy(&h_min, d_min, sizeof(int), cudaMemcpyDeviceToHost));
	HANDLE_ERROR(cudaMemcpy(&h_max, d_max, sizeof(int), cudaMemcpyDeviceToHost));

	printf("\nMinimum je %d ", h_min);
	printf("\nMaximum je %d ", h_max);

}
