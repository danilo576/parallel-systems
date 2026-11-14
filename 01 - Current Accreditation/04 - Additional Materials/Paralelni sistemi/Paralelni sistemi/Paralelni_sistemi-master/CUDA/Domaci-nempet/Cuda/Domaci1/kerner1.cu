#include "device_launch_parameters.h"
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <stdlib.h>
#include "string.h"
#include <math.h>
#include <cassert>
#include <iostream>
using namespace std;

#define N 20

void HANDLE_ERROR(cudaError_t call)
{
	cudaError_t ret = call;
	switch (ret)
	{
	case cudaSuccess:
		break;
	case cudaErrorInvalidValue:
		printf("ERROR: InvalidValue:%i.\n", __LINE__);
		exit(-1);
		break;
	case cudaErrorInvalidMemcpyDirection:
		printf("ERROR:Invalid memcpy direction:%i.\n", __LINE__);
		exit(-1);
		break;
	default:
		printf("ERROR>line:%i.%d' ‘ %s\n", __LINE__, ret, cudaGetErrorString(ret));
		exit(-1);
		break;
	}
}

__device__ int numCheck(int num){

	if(num < 0){
		return 0;
	}
	
	return num;

} 

__global__ void subMat( int* d_A, int* d_B, int* d_C ){

	C[threadIdX.y * N + threadIdX.x] = numCheck(A[threadIdX.y * N + threadIdX.x] - B[threadIdX.y * N + threadIdX.x]);
	
}

void main()
{
	int *h_A, *d_A,*h_B, *d_B, *h_C, *d_C,;

	h_A = (int*)malloc(N * sizeof(int));
	h_B = (int*)malloc(N * sizeof(int));

	for (int i = 0; i < N; i++)
		h_A[i] = rand() % 10;
		
	for (int i = 0; i < N; i++)
		h_B[i] = rand() % 10;

	
	cout << endl;

	HANDLE_ERROR(cudaMalloc((void**)&d_A, N * sizeof(int)));
	HANDLE_ERROR(cudaMalloc((void**)&d_B, N * sizeof(int)));
	HANDLE_ERROR(cudaMalloc((void**)&d_C, N * sizeof(int)));
	
	HANDLE_ERROR(cudaMemcpy(d_A, h_A, N * sizeof(int), cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(d_B, h_B, N * sizeof(int), cudaMemcpyHostToDevice));
	
	dim3 dimThreads( N , N  )
	
	kernel <<<1, dimThreads>>>(d_A, d_B, d_C);

	HANDLE_ERROR(cudaMemcpy(&h_C, d_C, N * sizeof(int), cudaMemcpyDeviceToHost));

	
	for (int i = 0; i < N; i++)
		cout << h_C[i] << " " <<  endl;
	

}