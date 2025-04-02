
#include "hgl_tq.h"

#include <stdio.h>
#include <time.h>

typedef HglThreadQueue(int, 128) TQueue;

static _Atomic int sum = 0;

void *worker(void *arg);

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

int main(void) {
    TQueue tq;
    hgl_tq_init(&tq);

    #define N 1000

    pthread_t t[N];
    for (int i = 0; i < N; i++) {
        assert(0 == pthread_create(&t[i], NULL, worker, (void *)&tq));
    }

    for (int i = 1; i <= 1000; i++) {
        hgl_tq_push(&tq, i);
    }

    hgl_tq_wait_until_idle(&tq, N);
    
    assert(sum == 500500);

    for (int i = 0; i < N; i++) {
        hgl_tq_push(&tq, -1);
    }

    hgl_tq_wait_until_empty(&tq);
    hgl_tq_destroy(&tq);
}

