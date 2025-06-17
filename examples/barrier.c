//#include <stdio.h>

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

int main(void)
{

    A = malloc(PROBLEM_SIZE*sizeof(*A));
    B = malloc(PROBLEM_SIZE*sizeof(*B));
    C = calloc(PROBLEM_SIZE*sizeof(*C), 1);

    for (int i = 0; i < PROBLEM_SIZE; i++) {
        A[i] = i;
        B[i] = i*2;
    }

    barrier = hgl_barrier_make(N_WORKERS);
    HglWorkerPool *wp = hgl_worker_pool_init(N_WORKERS, 32);
    for (int i = 0; i < N_WORKERS; i++) {
        hgl_worker_pool_add_job(wp, my_func, (void *)(uintptr_t)i);
    }
    hgl_worker_pool_wait(wp);
    hgl_worker_pool_destroy(wp);

    printf("sum = %lld\n", sum);

    return 0;
}
