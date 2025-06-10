
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

TEST(test_barrier, .timeout = 5)
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

    barrier = hgl_barrier_init(N_WORKERS);
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

    LOG("sum = %lld\n", sum);
    ASSERT(sum == 1649265868800ll);
}
