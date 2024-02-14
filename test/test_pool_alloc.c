#include <stdio.h>

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

#define HGL_POOL_ALIGNMENT 1024
#define HGL_POOL_ALLOC_IMPLEMENTATION
#include "hgl_pool_alloc.h"

static uint8_t memory_chunk[64];

static HglPool s_pool;

void *pool_alloc()
{
    return hgl_pool_alloc(&s_pool);
}

void pool_free(void *ptr)
{
    hgl_pool_free(&s_pool, ptr);
}

int main()
{
    s_pool = hgl_pool_make(8, sizeof(int));

    int *i0 = pool_alloc();
    int *i1 = pool_alloc();
    int *i2 = pool_alloc();
    int *i3 = pool_alloc();
    int *i4 = pool_alloc();
    int *i5 = pool_alloc();
    int *i6 = pool_alloc();
    int *i7 = pool_alloc();

    pool_free(i3);

    int *i8 = pool_alloc();

    printf("%p\n", (void *) i0);
    printf("%p\n", (void *) i1);
    printf("%p\n", (void *) i2);
    printf("%p\n", (void *) i3);
    printf("%p\n", (void *) i4);
    printf("%p\n", (void *) i5);
    printf("%p\n", (void *) i6);
    printf("%p\n", (void *) i7);
    printf("%p\n", (void *) i8);

    hgl_pool_destroy(&s_pool);

    s_pool = hgl_pool_make_from_buffer(memory_chunk, sizeof(memory_chunk), sizeof(int));
    printf("pool n chunks: %zu\n", s_pool.n_chunks);


    i0 = pool_alloc();
    i1 = pool_alloc();
    i2 = pool_alloc();
    i3 = pool_alloc();
    i4 = pool_alloc();

    pool_free(i3);
    i5 = pool_alloc();

    printf("%p\n", (void *) i0);
    printf("%p\n", (void *) i1);
    printf("%p\n", (void *) i2);
    printf("%p\n", (void *) i3);
    printf("%p\n", (void *) i4);
    printf("%p\n", (void *) i5);

    hgl_memdbg_report();
}
