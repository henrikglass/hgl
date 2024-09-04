#ifndef HGL_THREADPOOL_H
#define HGL_THREADPOOL_H

/*--- Include files ---------------------------------------------------------------------*/

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef void (*HglThreadFunc)(void *);

typedef struct
{
    HglThreadFunc func;
    void *arg;
} HglThreadPoolJob;

typedef struct
{
    /* workers */
    pthread_t *workers;
    uint32_t n_workers;
    _Atomic uint32_t n_alive_workers;
    _Atomic uint32_t n_busy_workers;
    _Atomic bool kill_workers;

    /* job queue */
    HglThreadPoolJob *job_queue;
    uint32_t jq_read_idx;
    uint32_t jq_write_idx;
    uint32_t jq_capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_notify_worker; // events: 1. job added to job queue 2. kill all workers
    pthread_cond_t cvar_notify_host;   // events: 1. job removed from job queue 2. worker became idle
} HglThreadPool;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

HglThreadPool *hgl_threadpool_init(uint32_t n_workers, uint32_t job_queue_capacity);
void hgl_threadpool_wait(HglThreadPool *threadpool);
void hgl_threadpool_destroy(HglThreadPool *threadpool);
void hgl_threadpool_add_job(HglThreadPool *threadpool, HglThreadFunc func, void *arg);

#endif /* HGL_THREADPOOL_H */

#ifdef HGL_THREADPOOL_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>
#include <sched.h> // DEBUG

// DEBUG
_Atomic int ctr = 0;

void *worker_(void *arg)
{
    HglThreadPool *tp = (HglThreadPool*) arg;
    HglThreadPoolJob job;

    srand(++ctr);
    int worker_id = rand() % 1000;

    tp->n_alive_workers++;
    printf("[%d] spawned\n", worker_id);

    while (true) {
        /* lock mutex */
        pthread_mutex_lock(&tp->mutex);

        /* wait till job queue becomes readable (or we get killed) */
        while(tp->jq_write_idx == tp->jq_read_idx) {
            pthread_cond_wait(&tp->cvar_notify_worker, &tp->mutex);

            if (tp->kill_workers) {
                pthread_mutex_unlock(&tp->mutex);
                goto kill;
            }
        }
            
        /* unque job & update read idx */
        job = tp->job_queue[tp->jq_read_idx];
        tp->jq_read_idx = (tp->jq_read_idx + 1) & (tp->jq_capacity - 1);

        /* signal that queue is now writable & unlock mutex */
        pthread_cond_signal(&tp->cvar_notify_host);
        pthread_mutex_unlock(&tp->mutex);

        /* mark worker as busy */
        tp->n_busy_workers++;

        /* do assigned job */
        (*job.func)(job.arg);
        
        /* mark worker as not busy */
        tp->n_busy_workers--;
        
        /* signal that there is one less busy worker */
        pthread_cond_signal(&tp->cvar_notify_host);
    }

kill:
    //printf("[%d] killed!\n", worker_id);
    //fflush(stdout);
    tp->n_alive_workers--;
    pthread_mutex_lock(&tp->mutex);
    pthread_cond_signal(&tp->cvar_notify_host);
    pthread_mutex_unlock(&tp->mutex);
    return NULL;
}

HglThreadPool *hgl_threadpool_init(uint32_t n_workers, uint32_t job_queue_capacity)
{

    assert(n_workers > 0);
    assert(job_queue_capacity > 1);
    assert((job_queue_capacity & (job_queue_capacity - 1)) == 0 && 
           "\'job_queue_capacity\' must be a power of 2");

    HglThreadPool *threadpool = malloc(sizeof(HglThreadPool));
    
    assert(threadpool != NULL && "Buy more RAM lol");

    *threadpool = (HglThreadPool) {
        .workers      = malloc(n_workers * sizeof(pthread_t)),
        .n_workers    = n_workers,
        .job_queue    = malloc(job_queue_capacity * sizeof(HglThreadPoolJob)),
        .jq_read_idx  = 0,
        .jq_write_idx = 0,
        .jq_capacity  = job_queue_capacity,
    };
    
    assert(threadpool->workers != NULL && "Buy more RAM lol");
    assert(threadpool->job_queue != NULL && "Buy more RAM lol");

    pthread_mutex_init(&threadpool->mutex, NULL);
    pthread_cond_init(&threadpool->cvar_notify_worker, NULL);
    pthread_cond_init(&threadpool->cvar_notify_host, NULL);

    for (uint32_t i = 0; i < threadpool->n_workers; i++) {
        pthread_create(&threadpool->workers[i], NULL, worker_, (void *) threadpool);        
    }
    
    return threadpool;
}

void hgl_threadpool_destroy(HglThreadPool *threadpool)
{
    HglThreadPool *tp = threadpool;

    /* assert: no pending jobs or busy workers */
    hgl_threadpool_wait(threadpool);

    pthread_mutex_lock(&tp->mutex);
    int c = 0;
    threadpool->kill_workers = true;
    while(threadpool->n_alive_workers != 0) {
        c++;
        pthread_cond_broadcast(&threadpool->cvar_notify_worker);
        //sched_yield();
        hgl_sleep_s(0.1);
        printf("host waiting...\n");
        pthread_cond_wait(&threadpool->cvar_notify_host, &threadpool->mutex);
        printf("host waiting complete\n");
    }
    pthread_mutex_unlock(&tp->mutex);

    printf("c = %d\n", c);

    for (uint32_t i = 0; i < threadpool->n_workers; i++) {
        pthread_join(threadpool->workers[i], NULL);
    }

    pthread_mutex_destroy(&threadpool->mutex);
    pthread_cond_destroy(&threadpool->cvar_notify_worker);
    pthread_cond_destroy(&threadpool->cvar_notify_host);

    free(threadpool->workers);
    free(threadpool->job_queue);
    free(threadpool);
}

void hgl_threadpool_wait(HglThreadPool *threadpool)
{
    pthread_mutex_lock(&threadpool->mutex);

    /* assert: no pending jobs in job queue */
    while(threadpool->jq_read_idx != threadpool->jq_write_idx) {
        pthread_cond_wait(&threadpool->cvar_notify_host, &threadpool->mutex);
    }

    /* assert: no busy workers */
    while (threadpool->n_busy_workers != 0) {
        pthread_cond_wait(&threadpool->cvar_notify_host, &threadpool->mutex);
    }

    pthread_mutex_unlock(&threadpool->mutex);
}

void hgl_threadpool_add_job(HglThreadPool *threadpool, HglThreadFunc func, void *arg)
{
    HglThreadPool *tp = threadpool;

    /* lock mutex */
    pthread_mutex_lock(&tp->mutex);

    /* wait till job queue becomes writable */
    while(((tp->jq_write_idx + 1) & (tp->jq_capacity - 1)) == tp->jq_read_idx) {
        pthread_cond_wait(&tp->cvar_notify_host, &tp->mutex);
    }

    /* queue job & update write idx */
    tp->job_queue[tp->jq_write_idx] = (HglThreadPoolJob) {.func = func, .arg = arg};
    tp->jq_write_idx = (tp->jq_write_idx + 1) & (tp->jq_capacity - 1);

    /* signal that queue is now readable & unlock mutex */
    pthread_cond_signal(&tp->cvar_notify_worker);
    pthread_mutex_unlock(&tp->mutex);
}

#endif
