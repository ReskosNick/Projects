/* File:     exercise1_3.c
 *
 * Purpose:  Parallel multipication of an upper triangular matrix M(nxn) and a Vector v(n). 
 *           A and V are initialized with random values between 0 and 1. Some changes to
 *           the multiplication are been made, in order to improve performance.
 * 
 * Compile:  make all (needs timer.h and my_rand.h)
 * 
 * Run:      make run ARGS="<thread_count> <n>"
 * 
 * Input:    Number of threads for the parallel approximation
 *           Dimension of M and v
 * 
 * Output:   Y: the product vector
 *           Elapsed time for the computation
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include "../timer.h"

/*Global variables*/
int thread_count, n;

/*Serial functions*/
void Usage (char* program_name);
void Get_args(int argc, char *argv[]);
void Gen_matrix(double M[]);
void Gen_vector(double v[]);
void Print_matrix(char *title, double M[]);
void Print_vector(char* title, double y[]);
const char* get_schedule_type(omp_sched_t schedule_type);
void output_csv(FILE *fp, omp_sched_t schedule_type, int chunk_size, double elapsed_time);

/* Parallel function */
double Omp_mat_vect(double M[], double x[], double y[]);

/*------------------------------------------------------------------*/
int main(int argc, char* argv[]){
    double *M, *x, *y, elapsed_time;

    /* Erase any previous values of the OMP_SCHEDULE environment 
       variable for the no_scheduling runs*/
    //unsetenv("OMP_SCHEDULE");

    Get_args(argc, argv);
    FILE *fp = fopen("Results1_3.csv", "a");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    /*csv records: schedule_type, chunk_size, Threads, n, Elapsed_time*/

    M = malloc(n*(n+1)/2 * sizeof(double));
    x = malloc(n*sizeof(double));
    y = malloc(n*sizeof(double));

    srand(time(NULL));
    Gen_matrix(M);
    Gen_vector(x);

    /* Setting the default scheduling if schedule(runtime) is not present*/
    // omp_set_schedule(omp_sched_static, 0);
    
    elapsed_time = Omp_mat_vect(M, x, y);
    omp_sched_t schedule_type;
    int chunk_size;
    omp_get_schedule(&schedule_type, &chunk_size);
    output_csv(fp, schedule_type, chunk_size, elapsed_time);

#ifdef DEBUG
    Print_matrix("Matrix M:", M);
    Print_vector("Vector v:", x);
    Print_vector("Product vector y:", y);
#endif

    free(M);
    free(x);
    free(y);

    return 0;
}

double Omp_mat_vect(double M[], double x[], double y[]) {
   int i, j, M_index;
   double start, finish, temp;

   GET_TIME(start);
#  pragma omp parallel for num_threads(thread_count) default(none) \
      private(i, j, temp, M_index) shared(M, x, y, n) schedule(runtime)
   for (i = 0; i < n; i++) {
        M_index = i * (n - (i-1)/2.0);
        temp = 0.0;
        for (j = i; j < n; j++)
            temp += M[M_index++] * x[j];
        y[i] = temp;
   }
   GET_TIME(finish);

   return finish - start;
}

void Gen_matrix(double M[]) {
   int i, j, M_index;
    for (i = 0; i < n; i++){
        M_index = i * (n - (i-1)/2.0);
        for (j = i; j < n; j++)
            M[M_index++] = random()/((double) RAND_MAX);
    }
}

void Gen_vector(double x[]) {
   int i;
   for (i = 0; i < n; i++)
      x[i] = random()/((double) RAND_MAX);
} 

void Print_matrix(char* title, double A[]) {
   int i, j;
   printf("\n%s\n", title);
   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) 
         printf("%.5f ", (j < i ? 0 : A[i*n+j - i*(i+1)/2]));
      printf("\n");
   }
}

void Print_vector(char* title, double y[]) {
   int i;
   printf("\n%s\n", title);
   for (i = 0; i < n; i++)
      printf("%.5f\n", y[i]);
}

void output_csv(FILE *fp, omp_sched_t schedule_type, int chunk_size, double elapsed_time) {
    const char *schedule_type_str = get_schedule_type(schedule_type);
    fprintf(fp, "%s,%d,%d,%d,%e\n", schedule_type_str, chunk_size, thread_count, n, elapsed_time);
}

const char* get_schedule_type(omp_sched_t schedule_type) {
    switch (schedule_type) {
        case omp_sched_dynamic:
            return "dynamic";
        case omp_sched_guided:
            return "guided";
        default:
            return "static";
    }
}


void Get_args(int argc, char* argv[]){
   if (argc != 3) Usage(argv[0]);
   thread_count= strtol(argv[1], NULL, 10);
   n = strtol(argv[2], NULL, 10);
   if (thread_count <= 0 || n <= 0) Usage(argv[0]);
}

void Usage (char* program_name) {
	fprintf(stderr, "Usage: %s <thread_count> <n>\n", program_name);
   	exit(0);
}
