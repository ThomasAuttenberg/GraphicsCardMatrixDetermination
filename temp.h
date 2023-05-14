#pragma once
#define _CRT_SECURE_NO_WARNINGS //��������� ������ ������
#include "device_launch_parameters.h" //���������� ��� ������������ ���������� �� ����������� ����������� 
NVIDIA.
#include <cuda_runtime.h> //���������� ��� ������ � CUDA
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <locale.h>
class temp
{

	cudaError_t err = cudaSuccess; //������������� ���������� err ���� cudaError_t � ���������� �� �������� 

		void check_err() { //������� ��� ����������� ������ ��� ���������� ���������� � �������������� CUDA
		if (err != cudaSuccess) { //�������� �� ������ ��� ������ ����������� ��������� ���������� 
				fprintf(stderr, "Failed ", cudaGetErrorString(err)); //���� ���������� ������, �� ��������� ����� 
				exit(EXIT_FAILURE); //���������� ������
		}
	}

	void getMatrix(int N, float* A) { //������� ��� ������������� ������� ������� N*N 
		for (int i = 0; i < N; i++) { //���� ��� ������� �� �������
			for (int j = 0; j < N; j++) //���� ��� ������� �� ��������
				A[i + j * N] = rand() % 10; //������� � ������ ������ ������� �������� �� 0 �� 9 
				A[i + N * N] = 0; //��������� ����� �������
		}
	}

	__global__ void gauss_stage1(float* a, int n, float x, int N) { //����������� ���� CUDA-�������, ������� ��������� ������ ������ ��������� ������ ��� ���������� ������� � ������������� ����(������ ��� �������
			int i = blockDim.x * blockIdx.x + threadIdx.x; //I ��� ����� �������� ������ CUDA, ����������� �� ������ ���������� blockDim, blockIdx � threadIdx
			if (i <= N - n + 1) { //������� �����������, ��� �������� ����������� ������ ��� ��������� �������, 
					a[n + N * (i + n)] /= x; //������� �������� ������� �� �������� X
			}
	}

	__global__ void gauss_stage2(float* a, int n, int i, float x, int N) { //����������� ���� CUDA-�������, ������� 
			int j = blockDim.x * blockIdx.x + threadIdx.x; //J ��� ����� �������� ������ CUDA, ����������� �� 
			if (j <= N - n - 1) { //��� ������� �����������, ��� �������� ����������� ������ ��� ��������� �������, 
				a[i + N * (j + n + 1)] -= a[n + N * (j + n + 1)] * x; //���� ������� ���������, �� ����������� 
			}
	}

	__global__ void gauss_stage3(float* a, int n, int N) { //����������� ���� CUDA-�������, ������� ��������� ������ 
			int i = blockDim.x * blockIdx.x + threadIdx.x; //I ��� ����� �������� ������ CUDA, ����������� �� ������ 
			if (i < n) { //��� ������� �����������, ��� �������� ����������� ������ ��� ��������� �������, 
					a[i + N * N] -= a[n + N * N] * a[i + n * N]; //����������� �������� ��������� �� �������� 
			}
	}


	void findDeterminant(int N) { //������� ��� ���������� ������������ ������� �� GPU � �������������� CUDA
		check_err(); //�������, ������� ��������� ������, ��������� ��� ���������� ��������� CUDA

		int threadsPerBlock = 128, //���������� ������� � �����.
		blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock, //���������� ������ � �����
		size = sizeof(float) * N * (N + 1); //������ ������� ������� � ������
		float* A = (float*)malloc(N * (N + 1) * sizeof(float)); //������, ���������� ������� � ������ ��������� 
		getMatrix(N, A); //�������, ������� ��������� ������� ���������� ������� �� 0 �� 9
		float* _A = NULL; //��������� �� ���������� ������ ��� ������� A �� GPU
		err = cudaMalloc((void**)&_A, size); check_err(); //��������� ������ �� GPU

		err = cudaMemcpy(_A, A, size, cudaMemcpyHostToDevice); check_err(); //����������� ������ �� ������� 

			for (int i = 0; i < N; i++) { //���� ����������� ������ ��� ����� ������ ������
				gauss_stage1 << <blocksPerGrid, threadsPerBlock >> > (_A, i, A[i + i * N], N); //����� ���� ��� 
					for (int j = i + 1; j < N; j++)
						gauss_stage2 << <blocksPerGrid, threadsPerBlock >> > (_A, i, j, A[j + i * N], N); //����� 
			}

		for (int i = N - 1; i > 0; i--) //���� ����������� ������ ���� ������ ������ � ����� -1
			gauss_stage3 << <blocksPerGrid, threadsPerBlock >> > (_A, i, N); //����� ���� ��� ���������� 
			
	
		double det = 1.0; //������������� ������������ �������

		for (int j = 0; j < N; j++) //���� ��� ������� ����������� �������
			det *= A[j + N * N]; //��������� ���� ��������� ��������� �������
		cudaFree(_A); //������������ ������ �� GPU
		free(A); //������������ ������ �� �����
	

	int main(void) {
		srand(time(NULL));
		const int size = 1000; //������������ ����������� �������
		FILE* text;
		for (int N = 2; N <= size; N++) { //���� ��� �������� ������ ������������ �� 2 �� N � ���������� �� 
				srand(time(NULL));
			int before = clock(); //������������� ���������� ��� ������ ������� ���������� ������������ 
			findDeterminant(N); //����� ������� ���������� ������������
			double time = (clock() - before) / (double)CLOCKS_PER_SEC; //�������� �������
			printf("Time: %.3f sec.\n", time); //����� ������ � ������� � �������
			text = fopen("GPU.txt", "a"); //������� ����� ��� ������ �������
			fprintf(text, "%d %.3f\n", N, time); //������ ������� � ����
			fclose(text); //�������� �����
		}
		return 0;
	}
};

