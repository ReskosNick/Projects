#ifndef RWLOCKS_H
#define RWLOCKS_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int active_readers;
    int waiting_readers;
    int active_writers;
    int waiting_writers;
    int write_priority;
} rw_lock;

typedef enum {
    READ_UNLOCK,
    WRITE_UNLOCK
} UnlockType;

typedef enum {
    PRIORITY_READERS,
    PRIORITY_WRITERS
} UnlockStrategy;

void init_rwlock(rw_lock* lock);
void destroy_rwlock(rw_lock* lock);
void read_lock(rw_lock* lock);
void write_lock(rw_lock* lock);
void rw_unlock(rw_lock* lock, UnlockType type, UnlockStrategy strategy);

#endif // RWLOCKS_H
