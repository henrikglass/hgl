//#include <stdio.h>

#include "hgl.h"

#define HGL_THREADPOOL_IMPLEMENTATION
#include "hgl_threadpool.h"

#include <stdio.h>

void debug_print_job_queue(HglThreadPool *threadpool)
{
    pthread_mutex_lock(&threadpool->mutex);

    uint32_t my_read_idx = threadpool->jq_read_idx;
    while (my_read_idx != threadpool->jq_write_idx) {
        HglThreadPoolJob job = threadpool->job_queue[my_read_idx]; 
        printf("job_queue[%u] = {func = %p, arg = %s}\n", 
               my_read_idx, job.func, (const char *) job.arg);
        my_read_idx = (my_read_idx + 1) & (threadpool->jq_capacity - 1);
    }
    
    pthread_mutex_unlock(&threadpool->mutex);
}

void my_func(void *arg)
{
    char *str = (char *) arg;

    /* wait between 0 and 1 seconds */
    srand((unsigned int)((uintptr_t)arg & 0xFFFFFFFFu));
    float sleep_time = (rand() % 100) / 100.0f;
    hgl_sleep_s(sleep_time);

    printf("%s\n", str);
    fflush(stdout);

}

int main(void)
{
    printf("Hello World!\n");

    HglThreadPool *tp = hgl_threadpool_init(4, 4);

    hgl_threadpool_add_job(tp, my_func, "This is job 1");
    hgl_threadpool_add_job(tp, my_func, "This is job 2");
    hgl_threadpool_add_job(tp, my_func, "This is job 3");
    hgl_threadpool_add_job(tp, my_func, "This is job 4");
    hgl_threadpool_add_job(tp, my_func, "This is job 5");
    hgl_threadpool_add_job(tp, my_func, "This is job 6");
    hgl_threadpool_add_job(tp, my_func, "This is job 7");
    hgl_threadpool_add_job(tp, my_func, "This is job 8");
    hgl_threadpool_add_job(tp, my_func, "This is job 9");

    //debug_print_job_queue(&tp);

    //hgl_sleep_s(100.0f);
    hgl_threadpool_wait(tp);
    printf("done waiting\n");

    hgl_threadpool_destroy(tp);
    printf("done destroying\n");
    
    //printf("sleeping\n");
    //hgl_sleep_s(100.0f);

    return 0;
}
