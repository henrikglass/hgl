
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
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
 * hgl_worker_pool.h implements a basic worker/thread pool library.
 *
 *
 * USAGE:
 *
 * Include hgl_worker_pool.h file like this:
 *
 *     #define HGL_WORKER_POOL_IMPLEMENTATION
 *     #include "hgl_worker_pool.h"
 *
 * Code example:
 *
 *     void my_task(void *arg)
 *     {
 *         char *str = (char *) arg;
 *         printf("%s\n", str);
 *     }
 *
 *          /.../
 *
 *     // create a worker pool with 8 workers (threads) and a job queue capacity of 4
 *     HglWorkerPool *wp = hgl_worker_pool_init(8, 4);
 *     if (wp == NULL) {
 *         return 1;
 *     }
 *
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #1");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #2");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #3");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #4");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #5");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #6");
 *     hgl_worker_pool_wait(wp); // wait till jobs #1 -- #6 are finished
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #7");
 *     hgl_worker_pool_add_job(wp, my_task, "This is job #8");
 *     hgl_worker_pool_wait(wp); // wait till jobs #7 -- #8 are finished
 *
 *     hgl_worker_pool_destroy(wp);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_WORKER_POOL_H
#define HGL_WORKER_POOL_H

/*--- Include files ---------------------------------------------------------------------*/

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    void (*func)(void *);
    void *arg;
} HglWorkerPoolJob;

typedef struct
{
    /* workers */
    pthread_t *workers;
    uint32_t n_workers;
    uint32_t n_alive_workers;
    uint32_t n_busy_workers;
    bool workers_keep_alive;

    /* job queue */
    HglWorkerPoolJob *job_queue;
    uint32_t jq_read_idx;
    uint32_t jq_write_idx;
    uint32_t jq_capacity;

    /* synchronization */
    pthread_mutex_t mutex;
    pthread_cond_t cvar_notify_worker; /* events: 1. job added to job queue     */
                                       /*         2. kill all workers           */
    pthread_cond_t cvar_notify_host;   /* events: 1. job removed from job queue */
                                       /*         2. worker became idle         */
                                       /*         3. worker was killed          */
} HglWorkerPool;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

/**
 * Initializes a worker pool with `n_workers` number of workers and a job queue
 * capacity of `job_queue_capacity` and returns a pointer to it.
 */
HglWorkerPool *hgl_worker_pool_init(uint32_t n_workers, uint32_t job_queue_capacity);

/**
 * Blocks the host thread until all workers belonging to `wp` are idle.
 */
void hgl_worker_pool_wait(HglWorkerPool *wp);

/**
 * Shuts down the workers belonging to `wp` and destroys `wp`.
 */
void hgl_worker_pool_destroy(HglWorkerPool *wp);

/**
 * Adds a job to the job queue of worker pool `wp`. `func` is function pointer
 * to a function that takes an argument of type `void *` and returns void.
 * `arg` is a void* that is passed to `func` upon being invoked.
 */
void hgl_worker_pool_add_job(HglWorkerPool *wp, void (*func)(void *), void *arg);

#endif /* HGL_WORKER_POOL_H */

#ifdef HGL_WORKER_POOL_IMPLEMENTATION

#include <stdlib.h> // abort
#include <stdio.h>
#include <assert.h>

#if !defined(HGL_WORKER_POOL_ALLOC) &&   \
    !defined(HGL_WORKER_POOL_FREE)
#  define HGL_WORKER_POOL_ALLOC malloc
#  define HGL_WORKER_POOL_FREE free
#endif

void *hgl_internal_worker_(void *arg);
void *hgl_internal_worker_(void *arg)
{
    HglWorkerPool *wp = (HglWorkerPool*) arg;
    HglWorkerPoolJob job;

    /* mark worker as alive & busy */
    pthread_mutex_lock(&wp->mutex);
    wp->n_alive_workers++;
    wp->n_busy_workers++;

    while (true) {

        /* wait till job queue becomes readable (or we get killed) */
        while(wp->jq_write_idx == wp->jq_read_idx) {
            /* mark worker as idle */
            wp->n_busy_workers--;

            pthread_cond_signal(&wp->cvar_notify_host);             /* SIGNAL: 2. worker became idle */
            pthread_cond_wait(&wp->cvar_notify_worker, &wp->mutex); /* AWAIT: 1. job added to queue OR 2. kill all workers */

            if (!wp->workers_keep_alive) {
                wp->n_alive_workers--;
                pthread_cond_signal(&wp->cvar_notify_host);         /* SIGNAL: 3. worker was killed */
                pthread_mutex_unlock(&wp->mutex);
                return NULL;
            }

            /* mark worker as busy */
            wp->n_busy_workers++;
        }

        /* unque job & update read idx */
        job = wp->job_queue[wp->jq_read_idx];
        wp->jq_read_idx = (wp->jq_read_idx + 1) & (wp->jq_capacity - 1);

        /* signal that queue is now writable & unlock mutex while performing job */
        pthread_cond_signal(&wp->cvar_notify_host); /* SIGNAL: 1. job removed from work queue */
        pthread_mutex_unlock(&wp->mutex);

        /* do assigned job */
        (*job.func)(job.arg);

        /* lock mutex */
        pthread_mutex_lock(&wp->mutex);

    }

}

HglWorkerPool *hgl_worker_pool_init(uint32_t n_requested_workers, uint32_t job_queue_capacity)
{

    if (n_requested_workers < 1) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: `n_requested_workers` can't be 0.\n");
        goto err;
    }

    if (job_queue_capacity < 2) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: `job_queue_capacity` can't be less than 2.\n");
        goto err;
    }

    if ((job_queue_capacity & (job_queue_capacity - 1)) != 0) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: `job_queue_capacity` must be a power of 2.\n");
        goto err;
    }

    HglWorkerPool *wp = HGL_WORKER_POOL_ALLOC(sizeof(HglWorkerPool));

    if (wp == NULL) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: memory allocation failed (worker pool).\n");
        goto err;
    }

    *wp = (HglWorkerPool) {
        .n_workers          = 0,
        .n_alive_workers    = 0,
        .n_busy_workers     = 0,
        .workers_keep_alive = true,

        .jq_read_idx        = 0,
        .jq_write_idx       = 0,
        .jq_capacity        = job_queue_capacity,
    };


    wp->workers = HGL_WORKER_POOL_ALLOC(n_requested_workers * sizeof(pthread_t));
    if (wp->workers == NULL) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: memory allocation failed (wp->workers).\n");
        goto err_free_threadpool;
    }

    wp->job_queue = HGL_WORKER_POOL_ALLOC(job_queue_capacity * sizeof(HglWorkerPoolJob));
    if (wp->job_queue == NULL) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: memory allocation failed (wp->job_queue).\n");
        goto err_free_workers;
    }

    int err = 0;
    err |= pthread_mutex_init(&wp->mutex, NULL);
    err |= pthread_cond_init(&wp->cvar_notify_worker, NULL);
    err |= pthread_cond_init(&wp->cvar_notify_host, NULL);
    if (err != 0) {
        fprintf(stderr, "[hgl_worker_pool_init] Error: Failed to initialize synchronization primitives. Aborting.\n");
        abort();
    }

    for (uint32_t i = 0; i < n_requested_workers; i++) {
        if (pthread_create(&wp->workers[i], NULL, hgl_internal_worker_, (void *) wp) != 0) {
            break;
        }
        wp->n_workers++;
    }

    if (wp->n_workers != n_requested_workers) {
        fprintf(stderr, "[hgl_worker_pool_init] Warning: Could only create %u (out of %u requested) workers. \n",
                wp->n_workers, n_requested_workers);
    }
    
    /* wait for all workers to come alive */
    pthread_mutex_lock(&wp->mutex);
    while ((wp->n_alive_workers != wp->n_workers) ||
           (wp->n_busy_workers) != 0) {
        pthread_cond_wait(&wp->cvar_notify_host, &wp->mutex);
    }
    pthread_mutex_unlock(&wp->mutex);

    return wp;

err_free_workers:
    HGL_WORKER_POOL_FREE(wp->workers);
err_free_threadpool:
    HGL_WORKER_POOL_FREE(wp);
err:
    return NULL;
}

void hgl_worker_pool_destroy(HglWorkerPool *wp)
{
    /* assert: no pending jobs or busy workers */
    hgl_worker_pool_wait(wp);

    /* Kill workers */
    pthread_mutex_lock(&wp->mutex);
    wp->workers_keep_alive = false;
    while(wp->n_alive_workers > 0) {
        pthread_cond_broadcast(&wp->cvar_notify_worker);
        pthread_cond_wait(&wp->cvar_notify_host, &wp->mutex);
    }
    pthread_mutex_unlock(&wp->mutex);

    /* joing worker threads */
    for (uint32_t i = 0; i < wp->n_workers; i++) {
        pthread_join(wp->workers[i], NULL);
    }

    /* finish up */
    pthread_mutex_destroy(&wp->mutex);
    pthread_cond_destroy(&wp->cvar_notify_worker);
    pthread_cond_destroy(&wp->cvar_notify_host);
    HGL_WORKER_POOL_FREE(wp->workers);
    HGL_WORKER_POOL_FREE(wp->job_queue);
    HGL_WORKER_POOL_FREE(wp);
}

void hgl_worker_pool_wait(HglWorkerPool *wp)
{
    /* lock mutex */
    pthread_mutex_lock(&wp->mutex);

    /* assert: no pending jobs in job queue */
    while(wp->jq_read_idx != wp->jq_write_idx) {
        pthread_cond_wait(&wp->cvar_notify_host, &wp->mutex); /* AWAIT: 1. job removed from job queue */
    }

    /* assert: no busy workers */
    while (wp->n_busy_workers != 0) {
        pthread_cond_wait(&wp->cvar_notify_host, &wp->mutex); /* AWAIT: 2. worker became idle */
    }

    /* unlock mutex */
    pthread_mutex_unlock(&wp->mutex);
}

void hgl_worker_pool_add_job(HglWorkerPool *wp, void (*func)(void *), void *arg)
{
    /* lock mutex */
    pthread_mutex_lock(&wp->mutex);

    /* wait till job queue becomes writable */
    while(((wp->jq_write_idx + 1) & (wp->jq_capacity - 1)) == wp->jq_read_idx) {
        pthread_cond_wait(&wp->cvar_notify_host, &wp->mutex);
    }

    /* queue job & update write idx */
    wp->job_queue[wp->jq_write_idx] = (HglWorkerPoolJob) {.func = func, .arg = arg};
    wp->jq_write_idx = (wp->jq_write_idx + 1) & (wp->jq_capacity - 1);

    /* signal that queue is now readable & unlock mutex */
    pthread_cond_signal(&wp->cvar_notify_worker); /* SIGNAL: 1. job added to job queue */
    pthread_mutex_unlock(&wp->mutex);
}

#endif
