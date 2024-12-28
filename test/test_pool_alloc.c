
#include "hgl_test.h"

#define HGL_POOL_ALIGNMENT 64
#define HGL_POOL_ALLOC_IMPLEMENTATION
#include "hgl_pool_alloc.h"

static HglPool pool;

GLOBAL_SETUP {
    pool = hgl_pool_make(16, 50); // 16 chunks of 50 bytes
}

GLOBAL_TEARDOWN {
    hgl_pool_destroy(&pool);
}

TEST(test_simple_alloc) {
    void *p0 = hgl_pool_alloc(&pool); 
    void *p1 = hgl_pool_alloc(&pool); 
    void *p2 = hgl_pool_alloc(&pool); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(p1 - p0 == 50);
    ASSERT(p2 - p1 == 50);

}

TEST(test_alignment) {
    ASSERT((((intptr_t)pool.memory) & (HGL_POOL_ALIGNMENT - 1)) == 0);
}

TEST(test_free_chunk) {
    void *p0 = hgl_pool_alloc(&pool); 
    void *p1 = hgl_pool_alloc(&pool); 
    void *p2 = hgl_pool_alloc(&pool); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_pool_free(&pool, p1);

    void *p3 = hgl_pool_alloc(&pool);

    ASSERT(p1 == p3);
}

TEST(test_free_all) {
    void *p0 = hgl_pool_alloc(&pool); 
    void *p1 = hgl_pool_alloc(&pool); 
    void *p2 = hgl_pool_alloc(&pool); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_pool_free_all(&pool);

    void *p3 = hgl_pool_alloc(&pool);

    ASSERT(p0 == p3);
}

TEST(test_too_many_allocs) {
    void *p0;
    for (int i = 0; i < 16; i++) {
        p0 = hgl_pool_alloc(&pool); 
    }
    ASSERT(p0 != NULL);
    p0 = hgl_pool_alloc(&pool); 
    ASSERT(p0 == NULL);
}
