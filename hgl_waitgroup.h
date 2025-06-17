
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
 * hgl_waitgroup.h implements a Go-style waitgroup.
 *
 *
 * EXAMPLE:
 *
 *     #define HGL_WAITGROUP_IMPLEMENTATION
 *     #include "hgl_waitgroup.h"
 *
 *     static HglWaitGroup wg;
 *
 *     void *f(void *arg) {
 *         // do work
 *         hgl_waitgroup_done(&wg);
 *     }
 *
 *     void g()
 *     {
 *         wg = hgl_waitgroup_make();
 *
 *         for (int i = 0; i < 16; i++) {
 *             hgl_waitgroup_add(&wg, 1);
 *             pthread_t t;
 *             pthread_create(&t, NULL, f, NULL);
 *             pthread_detach(t);
 *         }
 *
 *         hgl_waitgroup_wait(&wg);
 *         hgl_waitgroup_destroy(&wg);
 *     }
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_WAITGROUP_H
#define HGL_WAITGROUP_H

#include <pthread.h>

typedef struct
{
    int counter;
    pthread_mutex_t mutex;
    pthread_cond_t cvar;
} HglWaitGroup;

HglWaitGroup hgl_waitgroup_make(void);
void hgl_waitgroup_destroy(HglWaitGroup *wg);
void hgl_waitgroup_add(HglWaitGroup *wg, int n);
void hgl_waitgroup_done(HglWaitGroup *wg);
void hgl_waitgroup_wait(HglWaitGroup *wg);

#endif /* HGL_WAITGROUP_H */

#ifdef HGL_WAITGROUP_IMPLEMENTATION

HglWaitGroup hgl_waitgroup_make(void)
{
    int err;
    HglWaitGroup wg;

    wg.counter = 0;
    err  = pthread_mutex_init(&wg.mutex, NULL); 
    err |= pthread_cond_init(&wg.cvar, NULL); 
    assert (err == 0);

    return wg;
}

void hgl_waitgroup_destroy(HglWaitGroup *wg)
{
    int err;
    hgl_waitgroup_wait(wg);
    err  = pthread_mutex_destroy(&wg->mutex);
    err |= pthread_cond_destroy(&wg->cvar);
    assert(err == 0);
}

void hgl_waitgroup_add(HglWaitGroup *wg, int n)
{
    pthread_mutex_lock(&wg->mutex);
    wg->counter += n;
    pthread_mutex_unlock(&wg->mutex);
}

void hgl_waitgroup_done(HglWaitGroup *wg)
{
    pthread_mutex_lock(&wg->mutex);
    wg->counter--;
    if (wg->counter == 0) {
        pthread_cond_broadcast(&wg->cvar);
    } else if (wg->counter < 0) {
        fprintf(stderr, "[hgl_waitgroup] Error: waitgroup counter is < 0. Aborting...\n");
        abort();
    }
    pthread_mutex_unlock(&wg->mutex);
}

void hgl_waitgroup_wait(HglWaitGroup *wg)
{
    pthread_mutex_lock(&wg->mutex);
    while (wg->counter != 0) {
        pthread_cond_wait(&wg->cvar, &wg->mutex);
    }
    pthread_mutex_unlock(&wg->mutex);
}

#endif /* HGL_WAITGROUP_IMPLEMENTATION */
