#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <device_launch_parameters.h>
#include <cuda.h>
#include <cuda_runtime_api.h>

#define N 33
#define br_niti 5

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
  
__global__ void kernel(int* niz)
{
	int tid = threadIdx.x + blockIdx.x * blockDim.x;
	
	niz[tid] = niz[tid] * niz[tid];  //Za poziv kernela: kernel<<<N/br_niti + 1, br_niti>>>(d_A); u ovom slučaju svaka nit će obrađivati po jedan element niza. Broj elemenata niza ne sme biti veći od maksimalnog broja niti na GPU!
	
	/*Napomena:
	Ukoliko fiksiramo broj blokova i broj niti po bloku, svaka nit će morati da obrađuje više elemenata. U tom slučaju N može biti veće od maksimalnog mogućeg broja niti na GPU, pa je takvo rešenje sigurnije.
	U nastavku je dato takvo rešenje, a poziv kernela bi tada podrazumevao fiksiran broj blokova i broj niti:  kernel<<<br_blokova, br_niti>>>(d_A);
	while (tid < N)
	{
		niz[tid] = niz[tid] * niz[tid];
		tid += blockDim.x * gridDim.x;
	}	*/
	
}

int main(int argc, char* argv[])
{
	int *h_A, *d_A, *h_B;

	h_A = (int*)malloc(N * sizeof(int));

	for(int i = 0; i < N; i++)
		h_A[i] = rand() % 100;

	printf("Niz A:\n");
	for(int i = 0; i < N; i++)
		printf("%d ", h_A[i]);

	HANDLE_ERROR(cudaMalloc((void**)&d_A, N * sizeof(int)));

	HANDLE_ERROR(cudaMemcpy(d_A, h_A, N * sizeof(int), cudaMemcpyHostToDevice));

	kernel<<<N/br_niti + 1, br_niti>>>(d_A); 

	HANDLE_ERROR(cudaMemcpy(h_A, d_A, N * sizeof(int), cudaMemcpyDeviceToHost));

	printf("\nNovi niz A:\n");
		for (int i = 0; i < N; i++)
			printf("%d ", h_A[i]);
			
}

