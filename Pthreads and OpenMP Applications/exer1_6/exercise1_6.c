/* File:     exercise1_6.c
 *
 * Purpose:  Solve large linear systems via Gaussian elimination
 *           Serial or Parallel aproache is been applied
 * 
 * Compile:  make all  (needs timer.h, my_rand.h and rwlocks.h)
 * 
 * Usage:    make run ARGS="<thread_count> <linear_system_size> <approach>"
 * 
 * Input:    Number of threads
 *           Size n of the linear system
 *           Approach type: 0 for serial or 1 for parallel
 * 
 * Output:   The matrixes A, b and x
 * 			 Elapsed time to carry out the calculation of the Gaussian elimination and back substitution
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "../timer.h"

/*Global variables*/
int thread_count, n, approach;

/*Serial functions*/
void Usage(char *prog_name);
void Gen_matrix(double A[]);
void Gen_vector(double b[]);
void Get_args(int argc, char *argv[]);
void Print_matrix(double A[]);
void Print_vector(double y[]);
double Gauss_elim_serial(double A[], double b[]);
double Back_sub_serial(double A[], double b[], double x[]);
void output_csv(FILE *fp, char* functionality, double elapsed_time);

/*Parallel functions*/
double Gauss_elim_parallel(double A[], double b[]);
double Back_sub_parallel(double A[], double b[], double x[]);

int main(int argc, char* argv[]) {
    double elapsed_Gauss, elapsed_back;
    double *A, *b, *x;

    Get_args(argc, argv);
    FILE *fp = fopen("Results1_6.csv", "a");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    /*csv records: n, approach, thread_count, functionality, elapsed_time*/
    
    A = malloc(n*n*sizeof(double));
    b = malloc(n*sizeof(double));
    x = malloc(n*sizeof(double));   
    Gen_matrix(A);
    Gen_vector(b);
    #ifdef DEBUG
        printf("Matrix A: \n");
        Print_matrix(A);
        printf("Vector b: \n");
        Print_vector(b);     
        printf("Vector x: \n");
        Print_vector(x);
    #endif    

    if(approach == 0) {
        elapsed_Gauss = Gauss_elim_serial(A, b);
        output_csv(fp, "Gauss elimination", elapsed_Gauss);
        elapsed_back = Back_sub_serial(A, b, x);
        output_csv(fp, "Back substitution", elapsed_back);

        #ifdef DEBUG
            printf("Matrix A: \n");
            Print_matrix(A);
            printf("Vector b: \n");
            Print_vector(b);
        #endif  

    }
    else {       
        elapsed_Gauss=Gauss_elim_parallel(A, b);
        output_csv(fp, "Gauss elimination", elapsed_Gauss);
        elapsed_back=Back_sub_parallel(A, b, x);
        output_csv(fp, "Back substitution", elapsed_back);

        #ifdef DEBUG
            printf("Matrix A: \n");
            Print_matrix(A);
            printf("Vector b: \n");
            Print_vector(b);
        #endif    
        
    }

	fclose(fp);
    free(A);
    free(b);
    free(x);

    return 0;
}

void Gen_matrix(double A[]) {
   int i, j, temp;
   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        temp=random();
        while((temp==0)&&(i==j))
            temp=random();
        A[i*n+j] = (double) temp;
      }
   }
}

void Gen_vector(double b[]) {
   int i;
   for (i = 0; i < n; i++)
      b[i] = (double) random();
}

double Gauss_elim_serial(double A[], double b[]) {
    int i, j, k;
    double start, finish, ratio;
    GET_TIME(start);

    /*Gaussian elimination*/
    for (i = 0; i < n-1; i++) {
        for (j = i+1; j < n; j++) {
            ratio = A[j*n+i] / A[i*n+i];
            for (k = i; k < n; k++)
                A[j*n+k] -= (ratio * A[i*n+k]);
            b[j] -= (ratio*b[i]);
        }
    }

    GET_TIME(finish);
    return finish-start;
}

double Back_sub_serial(double A[], double b[], double x[]) {
    int row, col;
    double start, finish;
    GET_TIME(start);

    /*Back substitution*/
    for (row = n-1; row >= 0; row--) {
        x[row] = b[row];
        for (col = row+1; col < n; col++)
            x[row] -= A[row*n+col]*x[col];
        x[row] /= A[row*n+row];
    }
    GET_TIME(finish);
    return finish-start;
}

double Gauss_elim_parallel(double A[], double b[]) {
    int i, j, k;
    double start, finish, ratio;
    GET_TIME(start);

    /*Gaussian elimination*/      
    for (i = 0; i < n-1; i++) {
#      pragma omp parallel for num_threads(thread_count) \
            default(none) shared(A,n,b,i) private(j,k,ratio)                
            for (j = i+1; j < n; j++) {
                ratio = A[j*n+i] / A[i*n+i];
                for (k = i; k < n; k++)
                    A[j*n+k] -= (ratio * A[i*n+k]);
                b[j] -= (ratio*b[i]);
            }
    }

    GET_TIME(finish);
    return finish-start;
}

double Back_sub_parallel(double A[], double b[], double x[]) {
    int  row, col;
    double start, finish, temp;
    GET_TIME(start);

    /*Back substitution*/
    for (row = n-1; row >= 0; row--) {
        x[row] = b[row];
        temp=x[row];
#       pragma omp parallel for num_threads(thread_count) \
		    default(none) reduction(-: temp) private(col) shared(A, x, n, row)
        for (col = row+1; col < n; col++)
            temp -= A[row*n+col]*x[col];
        x[row]= temp / A[row*n+row];
    }

    GET_TIME(finish);
    return finish-start;
}

void Print_matrix(double A[]) {
   int   i, j;
   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
        printf("%4.1f ", A[i*n + j]);
      printf("\n");
   }
}  /* Print_matrix */

void Print_vector(double y[]) {
   int   i;
   for (i = 0; i < n; i++)
      printf("%4.1f ", y[i]);
   printf("\n");
}  /* Print_vector */

void output_csv(FILE *fp, char* functionality, double elapsed_time){
    fprintf(fp, "%d,%d,%d,%s,%e\n", n, approach, thread_count, functionality, elapsed_time);
}

void Get_args(int argc, char* argv[]) {
    if (argc != 4) {
        Usage(argv[0]);
    }
    thread_count = strtol(argv[1], NULL, 10);
    n = strtol(argv[2], NULL, 10);
    approach = strtol(argv[3], NULL, 10);
    if (thread_count <= 0 || n <= 0 || (approach != 0 && approach != 1)) {
        Usage(argv[0]);
    }
    if (approach == 0 && thread_count != 1) {
        fprintf(stderr, "Error: If Serial approach is chosen, there must be only one thread.\n");
        exit(EXIT_FAILURE);
    }
}

void Usage(char* program_name) {
    fprintf(stderr, "Usage: %s <thread_count> <linear_system_size> <approach>\n"
                    "       0 for Serial, 1 for Parallel\n"
                    "       <thread_count> must be positive\n"
                    "       <linear_system_size> must be positive\n"
                    "       <approach> must be either 0 or 1\n", program_name);
    exit(EXIT_FAILURE);
}
