
#include "hgl_test.h"

#define HGL_WAITGROUP_IMPLEMENTATION
#include "hgl_waitgroup.h"

#include <stdio.h>
#include <sys/types.h>

static HglWaitGroup wg;

static _Atomic int counter;

void *my_func(void *arg);
void *my_func(void *arg)
{
    int worker_idx = (uintptr_t)arg;

    printf("[%d] doing work.\n", worker_idx);
    sleep(1);
    counter++;
    printf("[%d] done.\n", worker_idx);
    hgl_waitgroup_done(&wg);

    return NULL;
}

TEST(test_waitgroup, .timeout = 2)
{
    wg = hgl_waitgroup_make(); 

    for (int i = 0; i < 420; i++) {
        hgl_waitgroup_add(&wg, 1);
        pthread_t t;
        pthread_create(&t, NULL, my_func, (void*)(uintptr_t)i);
        pthread_detach(t);
    }

    hgl_waitgroup_wait(&wg);

    ASSERT(counter == 420);

    hgl_waitgroup_destroy(&wg);
}
