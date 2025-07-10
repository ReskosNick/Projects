/* File:     exercise1_1.c
 *
 * Purpose:  Pi calculation via Monte Carlo method
 * 
 * Compile:  make all (needs timer.h and my_rand.h)
 * 
 * Run:      make run ARGS="<throw_num> <thread_count>"
 * 
 * Input:    Number of total throws
 *           Number of threads for the parallel approximation
 * 
 * Output:   Approximation of pi for every algorithm
 * 			 Elapsed time to carry out the respective calculation
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include "../my_rand.h"
#include "../timer.h"

/*Global variables*/
int thread_count;
long long int throw_num;
long long int total_cycle_throws_Pth;
long long int total_cycle_throws_OMP;
pthread_mutex_t mutex;

/* Serial functions */
double Serial();
void output_csv(FILE *fp, const char *algorithm, 
				double pi, double elapsed_time);
void Usage (char* program_name);
void Get_args(int argc, char *argv[]);

/* Parallel functions */
void* Parallel_Pth(void* rank);
long long int Parallel_OMP();

/*------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    double start, finish, elapsed;
	
	Get_args(argc, argv);
    FILE *fp = fopen("Results1_1.csv", "a");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	/*csv records: Algorithm, Throws, Threads, Pi, Elapsed_Time*/

	/*Serial Approach*/
	double pi_serial;

	GET_TIME(start);
	pi_serial = Serial();
	GET_TIME(finish);
	elapsed = finish - start;
	output_csv(fp, "Serial", pi_serial, elapsed);

	/*Parallel Approach with Pthreads*/
	long thread;
	double pi_Pth;
	pthread_t* thread_handles;
	
	thread_handles=malloc(thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);
	total_cycle_throws_Pth=0;
	GET_TIME(start);
	for (thread = 0; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, Parallel_Pth, (void*)thread);
	}
	for (thread = 0; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	pi_Pth = 4.0 * total_cycle_throws_Pth / ((double) throw_num);
	GET_TIME(finish);
	elapsed = finish - start;
	output_csv(fp, "Pthreads", pi_Pth, elapsed);
	free(thread_handles);
	pthread_mutex_destroy(&mutex);

	/*Parallel Approach with OpenMP*/
	double pi_OMP;

	total_cycle_throws_OMP = 0;
	GET_TIME(start);
	# pragma omp parallel num_threads(thread_count) \
	reduction(+: total_cycle_throws_OMP)
	{
	total_cycle_throws_OMP += Parallel_OMP();
	}
    pi_OMP = 4.0 * total_cycle_throws_OMP/((double) throw_num);
    GET_TIME(finish);
    elapsed = finish - start;
	output_csv(fp, "OpenMP", pi_OMP, elapsed);

	fclose(fp);

    return 0;
}

double Serial() {
	long long int cycle_throws = 0;
	long long int points;
	double x,y,r_squared,pi;
		for(points = 0; points < throw_num; points++){
			x = (rand() / ((double) RAND_MAX)) * 2 - 1;
			y = (rand() / ((double) RAND_MAX)) * 2 - 1;
			r_squared = x*x + y*y;
			if(r_squared <= 1.0){
				cycle_throws++;
			}
		}
		pi = 4.0*cycle_throws/((double) throw_num);
	return pi;
}

void* Parallel_Pth(void* rank) {
	long long int points;
	long long int my_cycle_throws=0;
	long long int my_hit_count = throw_num / thread_count;
	unsigned int seed;
	double x, y, r_squared;
	for(points=0; points < my_hit_count; points++) {
		seed = points + 1;
		x = -1.0 + 2.0 * my_drand(&seed);
		y = -1.0 + 2.0 * my_drand(&seed);
		r_squared = x*x + y*y;
		if(r_squared <= 1.0) {
			my_cycle_throws++;
		}
	}
	pthread_mutex_lock(&mutex);
	total_cycle_throws_Pth += my_cycle_throws;
	pthread_mutex_unlock(&mutex);
 	return NULL;
}

long long int Parallel_OMP() {
	long long int points;
	long long int my_cycle_throws=0;
	long long int my_hit_count = throw_num / thread_count;
	unsigned int seed;
	double x, y, r_squared;
	for(points=0; points < my_hit_count; points++) {
		seed = points + 1;
		x = -1.0+2.0*my_drand(&seed);
		y = -1.0+2.0*my_drand(&seed);
		r_squared = x*x + y*y;
		if(r_squared <= 1.0) {
			my_cycle_throws++;
		}
	}
 	return my_cycle_throws;
}

void output_csv(FILE *fp, const char *algorithm, double pi, double elapsed_time) {
    fprintf(fp, "%s,%lld,%d,%.10lf,%e\n", algorithm, throw_num, 
			thread_count, pi, elapsed_time);
}

void Get_args(int argc, char* argv[]){
   if (argc != 3) Usage(argv[0]);
   throw_num = strtol(argv[1], NULL, 10);
   thread_count= strtol(argv[2], NULL, 10);
   if (thread_count <= 0 || throw_num <= 0) Usage(argv[0]);
}

void Usage (char* program_name) {
	fprintf(stderr, "Usage: %s <throw_num> <thread_count> \n", program_name);
   	exit(0);
}