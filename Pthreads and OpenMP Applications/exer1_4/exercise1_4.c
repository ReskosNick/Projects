/* File:     exercise1_4.c
 *
 * Purpose:  Implement a multi-threaded sorted linked list of 
 *           ints with ops insert, print, member, delete, free list.  
 *           This version uses custom read-write locks.
 * 
 * Compile:  make all (needs timer.h, my_rand.h and rwlocks.h)
 *           
 * Run:      make run ARGS="<thread_count> <search_percent> <insert_percent> <approach>"
 *
 * Input:    total number of keys inserted by main thread
 *           total number of ops of each type carried out by each thread.
 *
 * Output:   Elapsed time to carry out the ops
 *
 * Notes:
 *    1.  Repeated values are not allowed in the list
 *    2.  DEBUG compile flag used.  To get debug output compile with
 *        -DDEBUG command line flag.
 *    3.  Uses the Unix 98 Standard implementation of read-write locks.
 *    4.  The random function is not threadsafe.  So this program
 *        uses a simple linear congruential generator.
 *    5.  -DOUTPUT flag to gcc will show list before and after
 *        threads have worked on it.
 *
 * IPP:   Section 4.9.3 (pp. 187 and ff.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../timer.h"
#include "../my_rand.h"
#include "rwlocks.h"

/* Random ints are less than MAX_KEY */
const int MAX_KEY = 100000000;


/* Struct for list nodes */
struct list_node_s {
   int    data;
   struct list_node_s* next;
};


/* Shared variables */
struct      list_node_s* head = NULL;  
int         thread_count;
int         total_ops;
double      insert_percent;
double      search_percent;
double      delete_percent;
rw_lock     rwlock;
pthread_mutex_t     count_mutex;

/* Setup and cleanup */
void        Usage(char* prog_name);
void        Get_input(int* inserts_in_main_p);
void        output_csv(FILE *fp, const char* label, double elapsed_time);

/* Thread functions for approaches A and B*/
void*       Thread_workA(void* rank);
void*       Thread_workB(void* rank);
void        run_parallel_approach(void* (*thread_func)(void*), 
                                             const char* label, FILE *fp);

/* List operations */
int         Insert(int value);
void        Print(void);
int         Member(int value);
int         Delete(int value);
void        Free_list(void);
int         Is_empty(void);

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   long i; 
   int key, success, attempts;
   int approach;
   int inserts_in_main;
   unsigned seed = 1;
   double start, finish, elapsed;

   FILE *fp = fopen("Results1_4.csv", "a");
   if (fp == NULL) {
      perror("Error opening file");
      exit(EXIT_FAILURE);
   }
   /*csv records: approach, threads, search_percent, insert_percent, delete_percent, elapsed_time */

   if (argc != 5) Usage(argv[0]);
   thread_count = strtol(argv[1],NULL,10);
   search_percent = strtod(argv[2],NULL);
   insert_percent = strtod(argv[3],NULL);
   approach = strtol(argv[4],NULL,10);
   delete_percent = 1.0 - (search_percent + insert_percent);

   inserts_in_main = 1000;
   total_ops = 500000;

   /* Try to insert inserts_in_main keys, but give up after */
   /* 2*inserts_in_main attempts.                           */
   i = attempts = 0;
   while ( i < inserts_in_main && attempts < 2*inserts_in_main ) {
      key = my_rand(&seed) % MAX_KEY;
      success = Insert(key);
      attempts++;
      if (success) i++;
   }
   printf("Inserted %ld keys in empty list\n", i);

#  ifdef OUTPUT
   printf("Before starting threads, list = \n");
   Print();
   printf("\n");
#  endif

if (approach == 0){
   /*Serial Approach*/
   int val;
   double which_op;

   GET_TIME(start);
   for (int k = 0; k < total_ops; k++) {
         seed= k;
         which_op = my_drand(&seed);
         val = my_rand(&seed) % MAX_KEY;
         if (which_op < search_percent) {
            Member(val);
         } else if (which_op < search_percent + insert_percent) {
            Insert(val);
         } else { 
            Delete(val);
         }
   }
   GET_TIME(finish);
   elapsed = finish - start;
	output_csv(fp, "Serial", elapsed);
   printf("Serial approach done in %e seconds\n", elapsed);

}  else if (approach == 1){
   run_parallel_approach(Thread_workA, "Read_first", fp);
}  else if (approach == 2){
   run_parallel_approach(Thread_workB, "Write_first", fp);
}  else Usage(argv[0]);

#  ifdef OUTPUT
   printf("After threads terminate, list = \n");
   Print();
   printf("\n");
#  endif
   Free_list();

   return 0;
}  /* main */


/* Function to run the parallel approach */
void run_parallel_approach(void* (*thread_func)(void*), const char* label, FILE *fp) {
    pthread_t* thread_handles = malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&count_mutex, NULL);
    init_rwlock(&rwlock);
    double start, finish, elapsed;

    GET_TIME(start);
    for (long i = 0; i < thread_count; i++) {
        pthread_create(&thread_handles[i], NULL, thread_func, (void*)i);
    }
    for (long i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }
    GET_TIME(finish);
    elapsed = finish - start;
    output_csv(fp, label, elapsed);

    printf("Parallel %s approach done in %e seconds\n", label, elapsed);

    destroy_rwlock(&rwlock);
    pthread_mutex_destroy(&count_mutex);
    free(thread_handles);
}

/*-----------------------------------------------------------------*/
void Usage(char* program_name) {
    fprintf(stderr, "Usage: %s <thread_count> <search_percent> <insert_percent> <approach>\n"
                    "       0 for Serial, 1 for Parallel A, 2 for Parallel B\n", program_name);
    exit(EXIT_FAILURE);
}  /* Usage */

/*-----------------------------------------------------------------*/
void Get_input(int* inserts_in_main_p) {

   printf("How many keys should be inserted in the main thread?\n");
   scanf("%d", inserts_in_main_p);
   printf("How many ops total should be executed?\n");
   scanf("%d", &total_ops);
   printf("Percent of ops that should be searches? (between 0 and 1)\n");
   scanf("%lf", &search_percent);
   printf("Percent of ops that should be inserts? (between 0 and 1)\n");
   scanf("%lf", &insert_percent);
   delete_percent = 1.0 - (search_percent + insert_percent);
}  /* Get_input */

/*-----------------------------------------------------------------*/
/* Insert value in correct numerical location into list */
/* If value is not in list, return 1, else return 0 */
int Insert(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   struct list_node_s* temp;
   int rv = 1;
   
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }

   if (curr == NULL || curr->data > value) {
      temp = malloc(sizeof(struct list_node_s));
      temp->data = value;
      temp->next = curr;
      if (pred == NULL)
         head = temp;
      else
         pred->next = temp;
   } else { /* value in list */
      rv = 0;
   }

   return rv;
}  /* Insert */

/*-----------------------------------------------------------------*/
void Print(void) {
   struct list_node_s* temp;

   printf("list = ");

   temp = head;
   while (temp != (struct list_node_s*) NULL) {
      printf("%d ", temp->data);
      temp = temp->next;
   }
   printf("\n");
}  /* Print */


/*-----------------------------------------------------------------*/
int  Member(int value) {
   struct list_node_s* temp;

   temp = head;
   while (temp != NULL && temp->data < value)
      temp = temp->next;

   if (temp == NULL || temp->data > value) {
#     ifdef DEBUG
      printf("%d is not in the list\n", value);
#     endif
      return 0;
   } else {
#     ifdef DEBUG
      printf("%d is in the list\n", value);
#     endif
      return 1;
   }
}  /* Member */

/*-----------------------------------------------------------------*/
/* Deletes value from list */
/* If value is in list, return 1, else return 0 */
int Delete(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   int rv = 1;

   /* Find value */
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }
   
   if (curr != NULL && curr->data == value) {
      if (pred == NULL) { /* first element in list */
         head = curr->next;
#        ifdef DEBUG
         printf("Freeing %d\n", value);
#        endif
         free(curr);
      } else { 
         pred->next = curr->next;
#        ifdef DEBUG
         printf("Freeing %d\n", value);
#        endif
         free(curr);
      }
   } else { /* Not in list */
      rv = 0;
   }

   return rv;
}  /* Delete */

/*-----------------------------------------------------------------*/
void Free_list(void) {
   struct list_node_s* current;
   struct list_node_s* following;

   if (Is_empty()) return;
   current = head; 
   following = current->next;
   while (following != NULL) {
#     ifdef DEBUG
      printf("Freeing %d\n", current->data);
#     endif
      free(current);
      current = following;
      following = current->next;
   }
#  ifdef DEBUG
   printf("Freeing %d\n", current->data);
#  endif
   free(current);
}  /* Free_list */

/*-----------------------------------------------------------------*/
int  Is_empty(void) {
   if (head == NULL)
      return 1;
   else
      return 0;
}  /* Is_empty */

/*-----------------------------------------------------------------*/
void* Thread_workA(void* rank) {
   long my_rank = (long) rank;
   int i, val;
   double which_op;
   unsigned seed = my_rank + 1;
   int ops_per_thread = total_ops/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      which_op = my_drand(&seed);
      val = my_rand(&seed) % MAX_KEY;
      if (which_op < search_percent) {
         read_lock(&rwlock);
         Member(val);
         rw_unlock(&rwlock, READ_UNLOCK, PRIORITY_READERS);
      } else if (which_op < search_percent + insert_percent) {
         write_lock(&rwlock);
         Insert(val);
         rw_unlock(&rwlock, WRITE_UNLOCK, PRIORITY_READERS);
      } else { /* delete */
         write_lock(&rwlock);
         Delete(val);
         rw_unlock(&rwlock, WRITE_UNLOCK, PRIORITY_READERS);
      }
   }  /* for */

   return NULL;
}  /* Thread_workA */

/*-----------------------------------------------------------------*/
void* Thread_workB(void* rank) {
   long my_rank = (long) rank;
   int i, val;
   double which_op;
   unsigned seed = my_rank + 1;
   int ops_per_thread = total_ops/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      which_op = my_drand(&seed);
      val = my_rand(&seed) % MAX_KEY;
      if (which_op < search_percent) {
         read_lock(&rwlock);
         Member(val);
         rw_unlock(&rwlock, READ_UNLOCK, PRIORITY_WRITERS);
      } else if (which_op < search_percent + insert_percent) {
         write_lock(&rwlock);
         Insert(val);
         rw_unlock(&rwlock, WRITE_UNLOCK, PRIORITY_WRITERS);
      } else { /* delete */
         write_lock(&rwlock);
         Delete(val);
         rw_unlock(&rwlock, WRITE_UNLOCK, PRIORITY_WRITERS);
      }
   }   /* for */

   return NULL;
}  /* Thread_workB */

/*-----------------------------------------------------------------*/

void  output_csv(FILE *fp, const char* label, double elapsed_time) {
    fprintf(fp, "%s,%d,%lf,%lf,%lf,%e\n", label, thread_count, search_percent,
                                          insert_percent, delete_percent, elapsed_time);
} /* output_csv*/