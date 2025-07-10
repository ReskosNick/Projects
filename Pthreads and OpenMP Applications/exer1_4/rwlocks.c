#include <pthread.h>


/* Struct for read-write lock*/
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int active_readers;
    int waiting_readers;
    int active_writers;
    int waiting_writers;
} rw_lock;

typedef enum {
    READ_UNLOCK,
    WRITE_UNLOCK
} UnlockType;

typedef enum {
    PRIORITY_READERS,
    PRIORITY_WRITERS
} UnlockStrategy;

void init_rwlock(rw_lock* lock) {
    lock->active_readers = 0;
    lock->waiting_readers = 0;
    lock->active_writers = 0;
    lock->waiting_writers = 0;
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->read_cond, NULL);
    pthread_cond_init(&lock->write_cond, NULL);
}

void destroy_rwlock(rw_lock* lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->read_cond);
    pthread_cond_destroy(&lock->read_cond);
}

void read_lock(rw_lock* lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->active_writers > 0) {
        lock->waiting_readers++;
        pthread_cond_wait(&lock->read_cond, &lock->mutex);
        lock->waiting_readers--;
    }
    lock->active_readers++;
    pthread_mutex_unlock(&lock->mutex);
}

void write_lock(rw_lock* lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->active_readers > 0 || lock->active_writers > 0) {
        lock->waiting_writers++;
        pthread_cond_wait(&lock->write_cond, &lock->mutex);
        lock->waiting_writers--;
    }
    lock->active_writers++;
    pthread_mutex_unlock(&lock->mutex);
}

void rw_unlock(rw_lock* lock, UnlockType type, UnlockStrategy strategy) {
    pthread_mutex_lock(&lock->mutex);

    if (type == READ_UNLOCK) {
        lock->active_readers--;
        if (lock->active_readers == 0 && lock->waiting_writers > 0) {
            pthread_cond_signal(&lock->write_cond);
        }
    } else if (type == WRITE_UNLOCK) {
        lock->active_writers--;
        if (strategy == PRIORITY_READERS) {
            if (lock->waiting_readers > 0) {
                pthread_cond_broadcast(&lock->read_cond);
            } else if (lock->waiting_writers > 0) {
                pthread_cond_signal(&lock->write_cond);
            }
        } else if (strategy == PRIORITY_WRITERS) {
            if (lock->waiting_writers > 0) {
                pthread_cond_signal(&lock->write_cond);
            } else if (lock->waiting_readers > 0) {
                pthread_cond_broadcast(&lock->read_cond);
            }
        }
    }

    pthread_mutex_unlock(&lock->mutex);
}
