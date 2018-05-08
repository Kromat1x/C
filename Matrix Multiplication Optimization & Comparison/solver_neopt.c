/*
 * Tema 2 ASC
 * 2018 Spring
 * Catalin Olaru / Vlad Spoiala
 */
#include "utils.h"

/*
 * Add your unoptimized implementation here
 */
double* my_solver(int N, double *A) {
	printf("NEOPT SOLVER\n");
	double *result = calloc(N * N * 2, sizeof(double));
	int i, j, k;
	
	for (i = 0; i < N; i++) {
		for (j = i; j < N; j++) {
			for (k = 0; k < N; k++) {
				result[2 * (i * N + j)] += A[2 * (i * N + k)] * A[2 * (j * N + k)] - A[2 * (i * N + k) + 1] * A[2 * (j * N + k) + 1];
				result[2 * (i * N + j) + 1] += A[2 * (i * N + k)] * A[2 * (j * N + k) + 1] + A[2 * (i * N + k) + 1] * A[2 * (j * N + k)];
			}
		}
	}
	
	return result;
}
