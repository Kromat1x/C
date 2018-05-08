/*
 * Tema 2 ASC
 * 2018 Spring
 * Catalin Olaru / Vlad Spoiala
 */
#include "utils.h"

/*
 * Add your optimized implementation here
 */
double* my_solver(int N, double *A) {
	printf("OPT_M SOLVER\n");
	double *result = calloc(N * N * 2, sizeof(double));
	register double real_sum, img_sum;
	int i, j, k, index_ij, i_N, j_N;
	
	for (i = 0; i < N; i++) {
		i_N = i * N;
		double *orig_ptr_real_1 = &A[2 * (i_N + 0)];
		double *orig_ptr_img_1 = &A[2 * (i_N + 0) + 1];
		
		for (j = i; j < N; j++) {
			j_N = j * N;
			index_ij = 2 * (i_N + j);
			
			double *ptr_real_1 = orig_ptr_real_1;
			double *ptr_img_1 = orig_ptr_img_1;
			double *ptr_real_2 = &A[2 * (j_N + 0)];
			double *ptr_img_2 = &A[2 * (j_N + 0) + 1]; 
			
			real_sum = 0.0;
			img_sum = 0.0;
			for (k = 0; k < N; k++) {
				real_sum += *ptr_real_1 * *ptr_real_2 - *ptr_img_1 * *ptr_img_2;
				img_sum += *ptr_real_1 * *ptr_img_2 + *ptr_img_1 * *ptr_real_2;
				ptr_real_1 += 2;
				ptr_real_2 += 2;
				ptr_img_1 += 2;
				ptr_img_2 += 2;
			}
			result[index_ij] = real_sum;
			result[index_ij + 1] = img_sum;
		}
	}
	
	return result;
}
