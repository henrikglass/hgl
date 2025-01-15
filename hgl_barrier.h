#ifndef HGL_BARRIER_H
#define HGL_BARRIER_H

#include <pthread.h>

typedef struct
{
    int n_threads;
    int n_waiting;
    pthread_mutex_t mutex;
    pthread_cond_t cvar;
} HglBarrier;

HglBarrier hgl_barrier_init(int n_threads);
void hgl_barrier_sync(HglBarrier *b);

#endif /* HGL_BARRIER_H */

#ifdef HGL_BARRIER_IMPLEMENTATION

#include <assert.h>

HglBarrier hgl_barrier_init(int n_threads)
{
    int err = 0;

    HglBarrier b = {
        .n_threads = n_threads,
        .n_waiting = 0
    };

    err  = pthread_mutex_init(&b.mutex, NULL);
    err |= pthread_cond_init(&b.cvar, NULL);
    assert(err == 0);

    return b;
}

void hgl_barrier_sync(HglBarrier *b)
{
    pthread_mutex_lock(&b->mutex);

    if (b->n_waiting == (b->n_threads - 1)) {
        b->n_waiting = 0;
        pthread_cond_broadcast(&b->cvar);
    } else {
        b->n_waiting++;
        while (b->n_waiting != 0) {
            pthread_cond_wait(&b->cvar, &b->mutex);
        }
    }

    pthread_mutex_unlock(&b->mutex);
}

#endif
