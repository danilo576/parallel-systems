#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <device_launch_parameters.h>
#include <cuda.h>
#include <cuda_runtime_api.h>

#define N 20
#define br_niti 4

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

__global__ void kernel(int* niz, int* poz, int* neg)
{
	int counter = 0;
	int tid = threadIdx.x; 
	__shared__ int numbers[br_niti];

	int slice = N / br_niti; //Koliko elemenata obradjuje svaka nit
	
	int start = tid * slice;
	int end = start + slice;
	if (tid == br_niti - 1) //Poslednja nit u bloku
		end += N % br_niti;
	
	for (int i = start; i < end; i++)
	{
		niz[i] = -niz[i]; //pristup globalnoj memoriji!
		if (niz[i] > 0)
			counter++;
	}
		
	//Princip redukcije! Referenca: Chapter 5.3, Jason Sanders, Edward Kandrot: CUDA by example: an introduction to general-purpose GPU programming, Addison-Wesley Professional
	
	numbers[tid] = counter;	
	
	__syncthreads();
	
	int half = br_niti;
	do{
		__syncthreads();
		half /= 2;
		if (tid < half)
		{
			numbers[tid] += numbers[tid+half];
		}
	} while(half!=1);
	
	if (tid == 0)
	{
		*poz = numbers[0];
		*neg = N - numbers[0];
	}
}

int main(){

	int *h_A, *d_A, *d_poz, *d_neg, h_poz, h_neg;
	
	h_A = (int*)malloc(N * sizeof(int));
	
	for (int i =0; i< N; i++)
		h_A[i] = rand() % 100 - 50;

	printf("Niz A:\n");
	for (int i =0; i< N; i++)	
		printf("%d ", h_A[i]);

	HANDLE_ERROR(cudaMalloc((void**)&d_A, N * sizeof(int)));
	
	HANDLE_ERROR(cudaMalloc((void**)&d_poz, sizeof(int)));
	
	HANDLE_ERROR(cudaMalloc((void**)&d_neg, sizeof(int)));

	HANDLE_ERROR(cudaMemcpy(d_A, h_A, N * sizeof(int), cudaMemcpyHostToDevice));

	kernel<<<1, br_niti>>>(d_A, d_poz, d_neg);

	HANDLE_ERROR(cudaMemcpy(h_A, d_A, N * sizeof(int), cudaMemcpyDeviceToHost));
	HANDLE_ERROR(cudaMemcpy(&h_neg, d_neg, sizeof(int), cudaMemcpyDeviceToHost));
	HANDLE_ERROR(cudaMemcpy(&h_poz, d_poz, sizeof(int), cudaMemcpyDeviceToHost));
	
	printf("\nNovi niz A:\n");
	for (int i =0; i< N; i++)	
		printf("%d ", h_A[i]);
		
	printf("\n Broj pozitivnih elemenata je %d ", h_poz);
	printf("\n Broj negativnih elemenata je %d ", h_neg);

}
