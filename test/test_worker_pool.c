
#include "hgl_test.h"

#define HGL_WORKER_POOL_IMPLEMENTATION
#include "hgl_worker_pool.h"

static HglWorkerPool *wp;

typedef struct
{
    int input;
    int result;
    const char *str;
} TaskData;

void square(void *arg)
{
    TaskData *task = (TaskData *) arg;
    task->result = task->input * task->input;
}

void do_nothing_fast(void *arg)
{
    (void) arg;
}

void setup()
{
    wp = hgl_worker_pool_init(8, 4);
}

void teardown()
{
    hgl_worker_pool_destroy(wp);
}

TEST(test_work1, .setup = setup, .teardown = teardown)
{
    LOG("This test should not halt.\n");
    TaskData tasks[] = {
        (TaskData) {.input = 1},
        (TaskData) {.input = 2},
        (TaskData) {.input = 3},
        (TaskData) {.input = 4},
        (TaskData) {.input = 5},
    };

    for (size_t i = 0; i < sizeof(tasks)/sizeof(tasks[0]); i++) {
        hgl_worker_pool_add_job(wp, square, (void *) &tasks[i]);
    }

    hgl_worker_pool_wait(wp);

    int sum = 0;
    for (size_t i = 0; i < sizeof(tasks)/sizeof(tasks[0]); i++) {
        sum += tasks[i].result;
    }

    ASSERT(sum == (1 + 4 + 9 + 16 + 25));

    hgl_worker_pool_wait(wp);
}

TEST(test_do_nothing_fast, .setup = setup, .teardown = teardown)
{
    LOG("This test should not halt.\n");
    for (size_t i = 0; i < 5; i++) {
        hgl_worker_pool_add_job(wp, do_nothing_fast, NULL);
    }
}

TEST(test_workers_alive_after_init)
{
    LOG("This test should not halt.\n");
    for (size_t i = 0; i < 1000; i++) {
        wp = hgl_worker_pool_init(8, 4);
        ASSERT(wp->n_alive_workers == 8);
        ASSERT(wp->n_workers == 8);
        ASSERT(wp->n_busy_workers == 0);
        hgl_worker_pool_destroy(wp);
    }
}

TEST(test_workers_waitable)
{
    LOG("This test should not halt.\n");
    for (size_t i = 0; i < 1000; i++) {
        wp = hgl_worker_pool_init(8, 4);
        ASSERT(wp->n_alive_workers == 8);
        ASSERT(wp->n_workers == 8);
        ASSERT(wp->n_busy_workers == 0);
        for (size_t i = 0; i < 5; i++) {
            hgl_worker_pool_add_job(wp, do_nothing_fast, NULL);
        }
        hgl_worker_pool_wait(wp);
        hgl_worker_pool_destroy(wp);
    }
}

TEST(test_workers_killable)
{
    LOG("This test should not halt.\n");
    for (size_t i = 0; i < 1000; i++) {
        wp = hgl_worker_pool_init(8, 4);
        ASSERT(wp->n_alive_workers == 8);
        ASSERT(wp->n_workers == 8);
        ASSERT(wp->n_busy_workers == 0);
        for (size_t i = 0; i < 5; i++) {
            hgl_worker_pool_add_job(wp, do_nothing_fast, NULL);
        }
        hgl_worker_pool_destroy(wp);
    }
}
