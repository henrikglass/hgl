
#include "hgl_test.h"

#define HGL_TQ_USE_HYBRID_MUTEX
#include "hgl_tq.h"

typedef HglThreadQueue(int, 128) TQueue;

static _Atomic int sum = 0;

void *worker(void *arg);
void *slow_worker(void *arg);

void *worker(void *arg)
{
    TQueue *tq = (TQueue *) arg;
    for (;;) {
        int i = hgl_tq_pop(tq, int);
        if (i == -1) break;
        sum += i;
    }

    return NULL;
}

void *slow_worker(void *arg)
{
    TQueue *tq = (TQueue *) arg;
    for (;;) {
        int i = hgl_tq_pop(tq, int);
        struct timespec t = {
            .tv_sec = 0,
            .tv_nsec = 100000000,
        };
        nanosleep(&t, &t);
        sum += i;
    }
}

TEST(single_worker) {
    TQueue tq;
    hgl_tq_init(&tq);

    pthread_t t;
    pthread_create(&t, NULL, worker, (void *)&tq);

    for (int i = 1; i <= 1000; i++) {
        hgl_tq_push(&tq, i);
    }

    hgl_tq_wait_until_idle(&tq, 1);

    ASSERT(sum == 500500);
}

TEST(single_slow_worker) {
    TQueue tq;
    hgl_tq_init(&tq);

    pthread_t t;
    pthread_create(&t, NULL, slow_worker, (void *)&tq);

    for (int i = 1; i <= 10; i++) {
        hgl_tq_push(&tq, i);
    }

    hgl_tq_wait_until_idle(&tq, 1);
    ASSERT(sum == 55);
}

#define N_THREADS 1000

TEST(multiple_workers_single_queue) {
    TQueue tq;
    hgl_tq_init(&tq);


    pthread_t t[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {
        ASSERT(0 == pthread_create(&t[i], NULL, worker, (void *)&tq));
        ASSERT(0 == pthread_detach(t[i]));
    }

    for (int i = 1; i <= 1000; i++) {
        hgl_tq_push(&tq, i);
    }

    hgl_tq_wait_until_idle(&tq, N_THREADS);

    ASSERT(sum == 500500);

    // too tired to explain. Not doing this can cause errors
    for (int i = 0; i < N_THREADS; i++) {
        hgl_tq_push(&tq, -1);
    }
    hgl_tq_wait_until_empty(&tq);
    hgl_tq_destroy(&tq);
}

TEST(multiple_workers_multiple_queues) {

    TQueue tq[N_THREADS];
    pthread_t t[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {
        hgl_tq_init(&tq[i]);
        pthread_create(&t[i], NULL, worker, (void *)&tq[i]);
    }

    for (int i = 1; i <= 1000; i++) {
        hgl_tq_push(&tq[i%N_THREADS], i);
    }

    for (int i = 0; i < N_THREADS; i++) {
        hgl_tq_wait_until_idle(&tq[i], 1);
    }

    ASSERT(sum == 500500);
}
