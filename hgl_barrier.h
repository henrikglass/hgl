
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_barrier.h implements a simple to use barrier synchronization primitive.
 *
 *
 * EXAMPLE:
 *
 *     #define HGL_BARRIER_IMPLEMENTATION
 *     #include "hgl_barrier.h"
 *
 *     #define N_THREADS 16
 *
 *     static HglBarrier barrier;
 *
 *     void *f(void *arg) {
 *         // 1. do work
 *         hgl_barrier_sync(&barrier); // syncronize with other threads
 *         // 2. do work that depends on 1. being completed
 *         hgl_barrier_sync(&barrier); // syncronize
 *         // 3. do work that depends on 2. being completed
 *         hgl_barrier_sync(&barrier); // syncronize
 *         // 4. do work that depends on 3. being completed
 *     }
 *
 *     void g()
 *     {
 *         pthread_t threads[N_THREADS];
 *         barrier = hgl_barrier_make(N_THREADS);
 *
 *         for (int i = 0; i < N_THREADS; i++) {
 *             pthread_create(&threads[i], NULL, f, NULL);
 *         }
 *
 *         for (int i = 0; i < N_THREADS; i++) {
 *             pthread_join(threads[i])
 *         }
 *         
 *         hgl_barrier_destroy(&barrier);
 *     }
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_BARRIER_H
#define HGL_BARRIER_H

#include <pthread.h>

typedef struct
{
    _Atomic int n_threads;
    unsigned int active_set;
    struct {
        int n_waiting;
        pthread_mutex_t mutex;
        pthread_cond_t cvar;
    } set[2];
} HglBarrier;

HglBarrier hgl_barrier_make(int n_threads);
void hgl_barrier_destroy(HglBarrier *b);
void hgl_barrier_add(HglBarrier *b, int n);
void hgl_barrier_sync(HglBarrier *b);

#endif /* HGL_BARRIER_H */

#ifdef HGL_BARRIER_IMPLEMENTATION

#include <assert.h>

HglBarrier hgl_barrier_make(int n_threads)
{
    int err = 0;
    HglBarrier b;
    b.n_threads = n_threads;
    b.active_set = 0;
    b.set[0].n_waiting = 0;
    b.set[1].n_waiting = 0;
    err  = pthread_mutex_init(&b.set[0].mutex, NULL);
    err |= pthread_mutex_init(&b.set[1].mutex, NULL);
    err |= pthread_cond_init(&b.set[0].cvar, NULL);
    err |= pthread_cond_init(&b.set[1].cvar, NULL);
    assert(err == 0);
    return b;
}

void hgl_barrier_destroy(HglBarrier *b)
{
    int err = 0;
    err  = pthread_mutex_destroy(&b->set[0].mutex);
    err |= pthread_mutex_destroy(&b->set[1].mutex);
    err |= pthread_cond_destroy(&b->set[0].cvar);
    err |= pthread_cond_destroy(&b->set[1].cvar);
    assert(err == 0);
}

void hgl_barrier_add(HglBarrier *b, int n)
{
    b->n_threads += n;
}

void hgl_barrier_sync(HglBarrier *b)
{
    pthread_mutex_t *mutex = &b->set[b->active_set].mutex;
    pthread_cond_t *cvar = &b->set[b->active_set].cvar;
    int *n_waiting = &b->set[b->active_set].n_waiting;

    pthread_mutex_lock(mutex);

    if (*n_waiting == (b->n_threads - 1)) {
        *n_waiting = 0;
        b->active_set ^= 1; // toggle active set
        pthread_cond_broadcast(cvar);
    } else {
        (*n_waiting)++;
        while (*n_waiting != 0) {
            pthread_cond_wait(cvar, mutex);
        }
    }

    pthread_mutex_unlock(mutex);
}

#endif /* HGL_BARRIER_IMPLEMENTATION */
