/* File:     exercise1_5.c
 *
 * Purpose:  Compare pthread mutexes with atomic instructions
 * 
 * Compile:  make all  (needs timer.h, my_rand.h)
 * 
 * Usage:    make run ARGS="<thread_count> <num_iter> <approach>"
 * 
 * Input:    Number of threads
 *           Number of iterations
 *           Approach type: 0 for mutexes or 1 for atomic instuctions
 * 
 * Output:   Elapsed time for the two approaches
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include "../timer.h"


/*Global variables*/
pthread_mutex_t mutex;
int thread_count, approach;
atomic_int shared_variable;
double start, finish, elapsed;
long long int total_iterations;

/* Serial functions */
void output_csv(FILE *fp, const char *algorithm, double elapsed_time);
void Usage (char* program_name);
void Get_args(int argc, char *argv[]);

/*Parallel Functions*/
void* mutex_lock(void* rank);
void* atomic(void* rank);



int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;

    Get_args(argc, argv);
    FILE *fp = fopen("Results1_5.csv", "a");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	/*csv records: Approach, total_iterations, thread_count, Elapsed_Time*/
    
    
    thread_handles=malloc(thread_count*sizeof(pthread_t));
	pthread_mutex_init(&mutex, NULL);
    shared_variable = 0;

    if (approach==0){
    /*mutex approach*/
        GET_TIME(start);
        for (thread = 0; thread < thread_count; thread++) {
            pthread_create(&thread_handles[thread], NULL, mutex_lock, (void*)thread);
        }
        
        for (thread = 0; thread < thread_count; thread++) {
            pthread_join(thread_handles[thread], NULL);
        }
        GET_TIME(finish);
        elapsed = finish - start;
        output_csv(fp, "mutex", elapsed);
        
    } else {
        /*atomic approach*/
        GET_TIME(start);
        for (thread = 0; thread < thread_count; thread++) {
            pthread_create(&thread_handles[thread], NULL, atomic, (void*)thread);
        }
        
        for (thread = 0; thread < thread_count; thread++) {
            pthread_join(thread_handles[thread], NULL);
        }
        GET_TIME(finish);
        elapsed = finish - start;
        output_csv(fp, "atomic", elapsed);
    }

	free(thread_handles);
	pthread_mutex_destroy(&mutex);
    fclose(fp);

    printf("Final value of shared variable: %d\n", shared_variable);
    
    return 0;
}

void* mutex_lock(void* rank){
	long long int my_iterations = total_iterations / thread_count;
    long long int i;

	for(i=0; i < my_iterations; i++) {
        pthread_mutex_lock(&mutex);
        shared_variable++;
        pthread_mutex_unlock(&mutex);
	}
 	return NULL;
}

void* atomic(void* rank) {
    long long int my_iterations = total_iterations / thread_count;
    long long int i;
    for (i = 0; i < my_iterations; i++) {
        atomic_fetch_add(&shared_variable, 1);
    }
    return NULL;
}

void output_csv(FILE *fp, const char *algorithm, double elapsed_time) {
    fprintf(fp, "%s,%lld,%d,%e\n", algorithm, total_iterations, 
			thread_count, elapsed_time);
}

void Get_args(int argc, char* argv[]){
   if (argc != 4) Usage(argv[0]);
   thread_count= strtol(argv[1], NULL, 10);
   total_iterations = strtol(argv[2], NULL, 10);
   approach = strtol(argv[3], NULL, 10);
   if (thread_count <= 0 || total_iterations <= 0) Usage(argv[0]);
    if (approach != 0 && approach != 1) {
    fprintf(stderr, "Error: Approach must be 0 for mutexes or 1 for atomic instructions.\n");
    exit(EXIT_FAILURE);
}
}

void Usage (char* program_name) {
	fprintf(stderr, "Usage: %s <throw_num> <total_iterations> \n", program_name);
   	exit(0);
}