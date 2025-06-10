//#include <stdio.h>

#define HGL_WORKER_POOL_IMPLEMENTATION
#include "hgl_worker_pool.h"

#include "hgl.h"

#include <stdio.h>

void debug_print_job_queue(HglWorkerPool *wp);
void debug_print_job_queue(HglWorkerPool *wp)
{
    pthread_mutex_lock(&wp->mutex);

    uint32_t my_read_idx = wp->jq_read_idx;
    while (my_read_idx != wp->jq_write_idx) {
        HglWorkerPoolJob job = wp->job_queue[my_read_idx];
        printf("job_queue[%u] = {func = ?, arg = %s}\n",
               my_read_idx, (const char *) job.arg);
        my_read_idx = (my_read_idx + 1) & (wp->jq_capacity - 1);
    }

    pthread_mutex_unlock(&wp->mutex);
}

void my_func(void *arg);
void my_func(void *arg)
{
    char *str = (char *) arg;

    /* wait between 0 and 0.5 seconds */
    srand((unsigned int)((uintptr_t)arg & 0xFFFFFFFFu));
    float sleep_time = (rand() % 100) / 200.0f;
    hgl_sleep_s(sleep_time);

    printf("%s\n", str);
    fflush(stdout);

}

int main(void)
{
    printf("Hello World!\n");

    HglWorkerPool *wp = hgl_worker_pool_init(24, 256);

    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  0");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  1");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  2");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  3");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  4");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  5");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  6");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  7");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  8");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job  9");
    hgl_worker_pool_wait(wp);
    printf("---------------\n");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 10");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 11");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 12");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 13");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 14");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 15");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 16");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 17");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 18");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 19");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 20");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 21");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 22");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 23");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 24");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 25");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 26");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 27");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 28");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 29");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 30");
    hgl_worker_pool_add_job(wp, my_func, (void *)"This is job 31");
    hgl_worker_pool_wait(wp);
    printf("---------------\n");

    hgl_worker_pool_destroy(wp);
    printf("main: done destroying\n");

    return 0;
}
