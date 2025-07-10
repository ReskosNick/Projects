/* File:     exercise1_2.c
 *
 * Purpose:  Parallel multipication of matrices A(mxn) and B(nxp). The matrices are initialized
 *           with random values between 0 and 1. Some small changes to
 *           the multiplication are been made, in order to improve performance.
 * 
 * Compile:  make all (needs timer.h and my_rand.h)
 * 
 * Run:      make run ARGS="<thread_count> <m> <n> <p>"
 * 
 * Input:    Number of threads for the parallel approximation
 *           Dimensions of the two matrices
 * 
 * Output:   Elapsed time for the initialization of the matrices A and B
 *           Y_fs: the product matrix with the problem of false sharing
 *           Y_mfs: the product matrix with the problem of false sharing minimized
 *           Elapsed time for the computation of each matrix
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../timer.h"
#include "../my_rand.h"

/* Global variables */
int     thread_count, thread_part;
int     m, n, p;
double  start, finish, elapsed;
double* A;
double* B;
double* BT;
double* Y_fs;
double* Y_mfs;
double** Y_mfs_part;


/*Serial functions*/
void Usage (char* program_name);
void Get_args(int argc, char *argv[]);
void Gen_matrix(double M[], int rows, int cols, char N);
void Print_matrix(char *title, double M[], int rows, int cols);
void Transpose(double MT[], double M[], int rows, int cols);
void Merge_matrices();
void output_csv(FILE *fp, const char *algorithm, double elapsed_time);

/* Parallel functions */
void *mat_mult_fs(void *rank);
void *mat_mult_mfs(void *rank);

/*------------------------------------------------------------------*/
int main(int argc, char* argv[]){
	long thread;
	
	Get_args(argc, argv);
    FILE *fp = fopen("Results1_2.csv", "a");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	/*csv records:  Algorithm, Threads, m, n, p, Elapsed_Time*/
	
    thread_part = m / thread_count;
    A  = malloc(m * n * sizeof(double));
	B  = malloc(n * p * sizeof(double));
	BT = malloc(n * p * sizeof(double));
	Y_fs  = malloc(m * p * sizeof(double));
	Y_mfs  = malloc(m * p * sizeof(double));
    Y_mfs_part = malloc(thread_count * sizeof(double*));

    srand(time(NULL));
	Gen_matrix(A, m, n, 'A');
	Gen_matrix(B, n, p, 'B');
	Transpose(BT, B, n, p);

	pthread_t *thread_handles = malloc(thread_count * sizeof(pthread_t));
	
	/*Algorithm with the problem of false sharing*/
	GET_TIME(start);
	for (thread = 0; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, mat_mult_fs, (void*)thread);
	}
	for (thread = 0; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	GET_TIME(finish);
	elapsed = finish - start;
	output_csv(fp, "FS", elapsed);
#ifdef DEBUG
	Print_matrix("The final matrix:", Y_fs, m, p);
	printf("\nThe final matrix was calculated in %e seconds.", elapsed);
#endif
	
	/*Algorithm with the problem of false sharing minimized*/
	GET_TIME(start);
	for (thread = 0; thread < thread_count; thread++) {
		Y_mfs_part[thread] = malloc(thread_part * p * sizeof(double));
		pthread_create(&thread_handles[thread], NULL, mat_mult_mfs, (void *) thread);
	}
	for (thread = 0; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	GET_TIME(finish);
	elapsed = finish - start;
	Merge_matrices();
	output_csv(fp, "MFS", elapsed);
#ifdef DEBUG
	Print_matrix("The final matrix with the problem of false sharing minimized:", Y_mfs, m, p);
	printf("\nThe final matrix with minimized false sharing was calculated in %e seconds.\n", elapsed);
#endif
		
	fclose(fp);
	free(A);
	free(B);
	free(BT);
	free(Y_fs);
    free(Y_mfs);
    for (thread = 0; thread < thread_count; thread++)
        free(Y_mfs_part[thread]);
    free(thread_handles);

	return 0;
}

void *mat_mult_fs(void *rank)
{
	int my_rank = (long) rank;
	int i, j, k;
	int my_first_row = my_rank * thread_part;
	int my_last_row = my_first_row + thread_part;
	int Y_index = my_first_row * p;
	register int A_index, B_index;
	double temp_sum, temp_prod;

	for (i = my_first_row; i < my_last_row; i++) {
		B_index = 0;
		for (j = 0; j < p; j++) {
			A_index = i * n;
			temp_sum = 0.0;
			for (k = 0; k < n; k++) {
				temp_prod = A[A_index++];
				temp_prod *= BT[B_index++];
				temp_sum += temp_prod;
				
			}
			Y_fs[Y_index++] = temp_sum;
		}
	}
	return NULL;
}

void *mat_mult_mfs(void *rank)
{
	int my_rank = (long) rank;
	int i, j, k;
	int my_first_row = my_rank * thread_part;
	int my_last_row = my_first_row + thread_part;
	int Y_index = 0;
	register int A_index, B_index;
	double temp_sum, temp_prod;

	for (i = my_first_row; i < my_last_row; i++) {
		B_index = 0;
		for (j = 0; j < p; j++) {
			A_index = i * n;
			temp_sum = 0.0;
			for (k = 0; k < n; k++) {
				temp_prod = A[A_index++];
				temp_prod *= BT[B_index++];
				temp_sum += temp_prod;
			}
			Y_mfs_part[my_rank][Y_index++] = temp_sum;
		}
	}

	return NULL;
} 

void Merge_matrices() {
	int thread;
	register int Y_index = 0;
	register int Y_part_index;

	for (thread = 0; thread < thread_count; thread++) {
		for (Y_part_index = 0; Y_part_index < thread_part * p; Y_part_index++)
			Y_mfs[Y_index++] = Y_mfs_part[thread][Y_part_index];
	}
}

void Transpose(double MT[], double M[], int rows, int cols)
{
	int i, j;
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			MT[j * rows + i] = M[i * cols + j];

}

void Gen_matrix(double M[], int rows, int cols, char N) {
   int i, j;
    GET_TIME(start);
    for (i = 0; i < rows; i++)
        for (j = 0; j < cols; j++)
            M[i * cols + j] = random()/((double) RAND_MAX);
    GET_TIME(finish);
    elapsed = finish - start;
#ifdef DEBUG
	printf("\nInitialization of matrix %c in %e seconds.", N, elapsed);
	Print_matrix("The generated matrix:", M, rows, cols);
#endif
} 

void Print_matrix(char *title, double M[], int rows, int cols)
{
	int i, j;

	printf("\n %s\n", title);
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++)
			printf("%6.3f ", M[i * cols + j]);
		printf("\n");
	}
}

void output_csv(FILE *fp, const char *algorithm, double elapsed_time) {
    fprintf(fp, "%s,%d,%d,%d,%d,%e\n", algorithm, thread_count, 
						m, n, p, elapsed_time);
}

void Get_args(int argc, char* argv[]){
   if (argc != 5) Usage(argv[0]);
   thread_count= strtol(argv[1], NULL, 10);
   m = strtol(argv[2], NULL, 10);
   n = strtol(argv[3], NULL, 10);
   p = strtol(argv[4], NULL, 10);
   if (thread_count <= 0 || m <= 0 || n <= 0 || p <= 0) Usage(argv[0]);
}

void Usage (char* program_name) {
	fprintf(stderr, "Usage: %s <thread_count> <m> <n> <p>\n", program_name);
   	exit(0);
}
