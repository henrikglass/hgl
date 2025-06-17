
#include "hgl_test.h"

#define HGL_WORKER_POOL_IMPLEMENTATION
#include "hgl_worker_pool.h"

#define HGL_BARRIER_IMPLEMENTATION
#include "hgl_barrier.h"

#include <stdio.h>
#include <sys/types.h>

#define N_WORKERS 16
#define PROBLEM_SIZE (1024*1024)
#define WORK_SIZE PROBLEM_SIZE/N_WORKERS

static HglBarrier barrier;

static int *A;
static int *B;
static int *C;
static long long int sum;

static _Atomic int counter1;
static _Atomic int counter2;
static _Atomic int counter3;
static _Atomic int counter4;
static _Atomic int counter5;
static _Atomic int counter6;

void my_func(void *arg);
void my_func(void *arg)
{
    int worker_idx = (uintptr_t)arg;
    for (int i = 0; i < WORK_SIZE; i++) {
        int idx = worker_idx*WORK_SIZE + i;
        C[idx] = A[idx] + B[idx];
    }

    hgl_barrier_sync(&barrier);

    if (worker_idx == 0) {
        sum = 0;
        for (int i = PROBLEM_SIZE - 1; i >= 0; i--) {
            sum += C[i];
        }
    }
}

void my_func2(void *arg);
void my_func2(void *arg)
{
    (void) arg;

    counter1++; // N_WORKERS
    hgl_barrier_sync(&barrier);
    counter2 += counter1; // N_WORKERS^2
    hgl_barrier_sync(&barrier);
    counter3 += counter2; // N_WORKERS^3
    hgl_barrier_sync(&barrier);
    counter4 += counter3; // N_WORKERS^4
    hgl_barrier_sync(&barrier);
    counter5 += counter4; // N_WORKERS^5
    hgl_barrier_sync(&barrier);
    counter6 += counter5; // N_WORKERS^6
}

TEST(test_barrier, .timeout = 10)
{
    A = malloc(PROBLEM_SIZE*sizeof(*A));
    B = malloc(PROBLEM_SIZE*sizeof(*B));
    C = calloc(PROBLEM_SIZE*sizeof(*C), 1);
    ASSERT(A != NULL);
    ASSERT(B != NULL);
    ASSERT(C != NULL);

    for (int i = 0; i < PROBLEM_SIZE; i++) {
        A[i] = i;
        B[i] = i*2;
    }

    barrier = hgl_barrier_make(N_WORKERS);
    HglWorkerPool *wp = hgl_worker_pool_init(N_WORKERS, 32);
    LOG("Doing 1000 iterations, should take no more than a couple seconds...");
    for (int i = 0; i < 1000; i++) {
        sum = 0;
        for (int j = 0; j < N_WORKERS; j++) {
            hgl_worker_pool_add_job(wp, my_func, (void *)(uintptr_t)j);
        }
        hgl_worker_pool_wait(wp);
        
        // https://www.wolframalpha.com/input?i=sum+i%2Bi*2%2C+i%3D0..1024*1024-1
        if (sum != 1649265868800ll) {
            LOG("wrong result on iteration #%d\n", i);
            LOG("got sum %lld, expected %lld\n", sum, 1649265868800ll);
            ASSERT(0);
        }
    }
    hgl_worker_pool_wait(wp);
    hgl_worker_pool_destroy(wp);

    hgl_barrier_destroy(&barrier);
    LOG("sum = %lld\n", sum);
    ASSERT(sum == 1649265868800ll);
}

TEST(test_multiple_barriers, .timeout = 1)
{
    barrier = hgl_barrier_make(N_WORKERS);
    HglWorkerPool *wp = hgl_worker_pool_init(N_WORKERS, 32);

    for (int j = 0; j < N_WORKERS; j++) {
        hgl_worker_pool_add_job(wp, my_func2, NULL);
    }
    hgl_worker_pool_wait(wp);

    LOG("counter1 = %d\n", counter1);
    LOG("counter2 = %d\n", counter2);
    LOG("counter3 = %d\n", counter3);
    LOG("counter4 = %d\n", counter4);
    LOG("counter5 = %d\n", counter5);
    LOG("counter6 = %d\n", counter6);
    ASSERT(counter1 == N_WORKERS);
    ASSERT(counter2 == N_WORKERS*counter1);
    ASSERT(counter3 == N_WORKERS*counter2);
    ASSERT(counter4 == N_WORKERS*counter3);
    ASSERT(counter5 == N_WORKERS*counter4);
    ASSERT(counter6 == N_WORKERS*counter5);

    hgl_barrier_destroy(&barrier);
    hgl_worker_pool_destroy(wp);
}
