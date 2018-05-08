/*
 * Tema 2 ASC
 * 2018 Spring
 * Catalin Olaru / Vlad Spoiala
 */
#include "utils.h"


/* 
 * Add your BLAS implementation here
 */
double* my_solver(int N, double *A) {
	printf("BLAS SOLVER\n");
	double *result = calloc(N * N * 2, sizeof(double));
	
	double alpha[] = {1, 0};
	double beta[] = {0, 0};
	
	cblas_zsyrk(CblasRowMajor, CblasUpper, CblasNoTrans, N, N, &alpha, A, N, &beta, result, N);
	return result;
}
