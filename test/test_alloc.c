
#include "hgl_test.h"

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"

static HglAllocator arena;
static HglAllocator stack_allocator;
static HglAllocator fs_allocator;
static HglAllocator pool;
static HglAllocator scratch;
static HglAllocator lmmalloc;

GLOBAL_SETUP {
    arena = hgl_alloc_make(.kind = HGL_ARENA_ALLOCATOR,  
                           .size = 128 * 1024, 
                           .alignment = 1024); // 128 KiB
    stack_allocator = hgl_alloc_make(.kind = HGL_STACK_ALLOCATOR, 
                                     .size = 128 * 1024, 
                                     .alignment = 1024); // 128 KiB
    fs_allocator = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR, 
                                  .size = 64*1024, 
                                  .free_stack_capacity = 16, 
                                  .alignment = 128);
    pool = hgl_alloc_make(.kind = HGL_POOL_ALLOCATOR, 
                          .size = 16*50, 
                          .pool_chunk_size = 50);
    scratch = hgl_alloc_make(.kind = HGL_SCRATCH_ALLOCATOR,
                             .size = 16,
                             .alignment = 4);
    lmmalloc = hgl_alloc_make(.kind = HGL_LIFETIME_MANAGED_MALLOC_ALLOCATOR);
}

GLOBAL_TEARDOWN {
    hgl_alloc_destroy(&arena);
    hgl_alloc_destroy(&stack_allocator);
    hgl_alloc_destroy(&fs_allocator);
    hgl_alloc_destroy(&pool);
}

/* HGL_ARENA_ALLOC */

TEST(arena_test_simple_alloc) {
    void *p0 = hgl_alloc(&arena, 100); 
    void *p1 = hgl_alloc(&arena, 1000); 
    void *p2 = hgl_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_alloc(&arena, 0) == NULL);
}

TEST(arena_test_free_all_behavior) {
    void *p0 = hgl_alloc(&arena, 100); 
    void *p1 = hgl_alloc(&arena, 1000); 
    void *p2 = hgl_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&arena);

    void *p3 = hgl_alloc(&arena, 100);

    ASSERT(p0 == p3);
}

TEST(arena_test_too_big_allocation) {
    void *p0 = hgl_alloc(&arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(arena_test_alignment) {
    void *p0 = hgl_alloc(&arena, 1); 
    void *p1 = hgl_alloc(&arena, 1); 
    void *p2 = hgl_alloc(&arena, 1); 

    ASSERT(p1 - p0 == arena.config.alignment);
    ASSERT(p2 - p1 == arena.config.alignment);
}

/* HGL_STACK_ALLOCATOR */

TEST(stack_test_realloc) {
    void *p0 = hgl_alloc(&stack_allocator, 10); 
    void *p1 = hgl_alloc(&stack_allocator, 20); 

    size_t previous_arena_head = stack_allocator.head;
    void *p2 = hgl_realloc(&stack_allocator, p1, 20);
    size_t new_arena_head = stack_allocator.head;

    ASSERT(previous_arena_head == new_arena_head);
}

TEST(
    stack_test_realloc_invalid_pointer, 
    .expect_signal = SIGABRT
) {
    void *p0 = hgl_alloc(&stack_allocator, 10); 
    void *p1 = hgl_alloc(&stack_allocator, 20); 
    void *p2 = hgl_realloc(&stack_allocator, p0, 20);
}

TEST(stack_simple_alloc) {
    void *p0 = hgl_alloc(&stack_allocator, 100); 
    void *p1 = hgl_alloc(&stack_allocator, 1000); 
    void *p2 = hgl_alloc(&stack_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_alloc(&stack_allocator, 0) == NULL);
}

TEST(stack_free_all_behavior) {
    void *p0 = hgl_alloc(&stack_allocator, 100); 
    void *p1 = hgl_alloc(&stack_allocator, 1000); 
    void *p2 = hgl_alloc(&stack_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&stack_allocator);

    void *p3 = hgl_alloc(&stack_allocator, 100);

    ASSERT(p0 == p3);
}

TEST(stack_too_big_allocation) {
    void *p0 = hgl_alloc(&stack_allocator, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(stack_realloc) {
    unsigned int *p0 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    unsigned int *p4 = hgl_realloc(&stack_allocator, p3, sizeof(unsigned int));
    ASSERT(p4 != NULL);
    ASSERT(p4 == p3);
}

TEST(stack_realloc_invalid_ptr, .expect_signal = SIGABRT) {
    unsigned int *p0 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_alloc(&stack_allocator, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    p3 = hgl_realloc(&stack_allocator, p1, sizeof(unsigned int));
}

TEST(stack_free_last) {
    void *p0 = hgl_alloc(&stack_allocator, 100); 
    void *p1 = hgl_alloc(&stack_allocator, 1000); 
    void *p2 = hgl_alloc(&stack_allocator, 10000); 

    hgl_free(&stack_allocator, p2);

    void *p3 = hgl_alloc(&stack_allocator, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}

TEST(stack_free_non_last, .expect_signal = SIGABRT) {
    void *p0 = hgl_alloc(&stack_allocator, 100); 
    void *p1 = hgl_alloc(&stack_allocator, 1000); 
    void *p2 = hgl_alloc(&stack_allocator, 10000); 

    hgl_free(&stack_allocator, p1);

    void *p3 = hgl_alloc(&stack_allocator, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}

/* HGL_FREE_STACK_ALLOCATOR */

TEST(fs_test_simple_alloc) {
    void *p0 = hgl_alloc(&fs_allocator, 100); 
    void *p1 = hgl_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_alloc(&fs_allocator, 0) == NULL);
}

TEST(fs_test_free_all_behavior) {
    void *p0 = hgl_alloc(&fs_allocator, 100); 
    void *p1 = hgl_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&fs_allocator);

    void *p3 = hgl_alloc(&fs_allocator, 100);

    ASSERT(p0 == p3);
}

TEST(fs_test_free_single_1) {
    void *p0 = hgl_alloc(&fs_allocator, 100); 
    void *p1 = hgl_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free(&fs_allocator, p0);

    void *p3 = hgl_alloc(&fs_allocator, 500);

    ASSERT(p0 != p3);

    void *p4 = hgl_alloc(&fs_allocator, 50);

    ASSERT(p0 == p4);

}

TEST(fs_test_free_single_2) {
    void *p0 = hgl_alloc(&fs_allocator, 100); 
    void *p1 = hgl_alloc(&fs_allocator, 1000); 
    void *p2 = hgl_alloc(&fs_allocator, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free(&fs_allocator, p1);

    void *p3 = hgl_alloc(&fs_allocator, 5000);

    ASSERT(p1 != p3);
    ASSERT(p0 != p3);

    void *p4 = hgl_alloc(&fs_allocator, 500);

    ASSERT(p1 == p4);
}

TEST(fs_test_free_multiple) {
    void *p0 = hgl_alloc(&fs_allocator, 100); 
    void *p1 = hgl_alloc(&fs_allocator, 100); 
    void *p2 = hgl_alloc(&fs_allocator, 100); 
    void *p3 = hgl_alloc(&fs_allocator, 100); 
    void *p4 = hgl_alloc(&fs_allocator, 100); 
    void *p5 = hgl_alloc(&fs_allocator, 100); 
    void *p6 = hgl_alloc(&fs_allocator, 100); 
    void *p7 = hgl_alloc(&fs_allocator, 100); 
    void *p8 = hgl_alloc(&fs_allocator, 100); 
    void *p9 = hgl_alloc(&fs_allocator, 100); 

    hgl_free(&fs_allocator, p1);
    hgl_free(&fs_allocator, p3);
    hgl_free(&fs_allocator, p5);
    hgl_free(&fs_allocator, p7);
    hgl_free(&fs_allocator, p9);

    ASSERT(hgl_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_alloc(&fs_allocator, 100) <= p9);
    ASSERT(hgl_alloc(&fs_allocator, 100) > p9);
}

TEST(fs_test_too_big_alloc) {
    void *p0 = hgl_alloc(&fs_allocator, 32*1024); 
    ASSERT(p0 != NULL);
    void *p1 = hgl_alloc(&fs_allocator, 24*1024); 
    ASSERT(p1 != NULL);
    void *p2 = hgl_alloc(&fs_allocator, 24*1024); 
    ASSERT(p2 == NULL);
}

TEST(fs_test_realloc) {
    void *p0 = hgl_alloc(&fs_allocator, 32); 
    char *p1 = hgl_alloc(&fs_allocator, 64); 
    void *p2 = hgl_alloc(&fs_allocator, 32); 

    ASSERT(p0 < (void*)p1);
    ASSERT((void*)p1 < p2);

    strcpy(p1, "Hejsan hoppsan");
    ASSERT(strcmp(p1, "Hejsan hoppsan") == 0);

    char *p3 = hgl_realloc(&fs_allocator, p1, 256);
    ASSERT(p3 != NULL);
    ASSERT(strcmp(p3, "Hejsan hoppsan") == 0);

    ASSERT(p2 < (void *)p3);
    void *p4 = hgl_alloc(&fs_allocator, 32);
    ASSERT(p4 < p2);
    ASSERT(p0 < p4);
}

/* HGL_POOL_ALLOCATOR */

TEST(pool_test_simple_alloc) {
    void *p0 = hgl_alloc_from_pool(&pool); 
    void *p1 = hgl_alloc_from_pool(&pool); 
    void *p2 = hgl_alloc(&pool, pool.config.pool_chunk_size); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(p1 - p0 == 50);
    ASSERT(p2 - p1 == 50);

}

TEST(pool_test_alignment) {
    ASSERT((((intptr_t)pool.memory) & (pool.config.alignment - 1)) == 0);
}

TEST(pool_test_free_chunk) {
    void *p0 = hgl_alloc_from_pool(&pool); 
    void *p1 = hgl_alloc_from_pool(&pool); 
    void *p2 = hgl_alloc_from_pool(&pool); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free(&pool, p1);

    void *p3 = hgl_alloc_from_pool(&pool);

    ASSERT(p1 == p3);
}

TEST(pool_test_free_all) {
    void *p0 = hgl_alloc_from_pool(&pool); 
    void *p1 = hgl_alloc_from_pool(&pool); 
    void *p2 = hgl_alloc_from_pool(&pool); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&pool);

    void *p3 = hgl_alloc_from_pool(&pool);

    ASSERT(p0 == p3);
}

TEST(pool_test_too_many_allocs) {
    void *p0;
    for (int i = 0; i < 16; i++) {
        p0 = hgl_alloc_from_pool(&pool); 
    }
    ASSERT(p0 != NULL);
    p0 = hgl_alloc_from_pool(&pool); 
    ASSERT(p0 == NULL);
}

/* HGL_SCRATCH_ALLOCATOR */

TEST(scratch_test_simple_alloc) {
    int *a[10];

    a[0] = hgl_alloc(&scratch, sizeof(int));
    a[1] = hgl_alloc(&scratch, sizeof(int));
    a[2] = hgl_alloc(&scratch, sizeof(int));
    a[3] = hgl_alloc(&scratch, sizeof(int));
    a[4] = hgl_alloc(&scratch, sizeof(int));
    a[5] = hgl_alloc(&scratch, sizeof(int));

    /* expect silent wraparound after 16 bytes */
    ASSERT(a[0] == a[4]);
    ASSERT(a[1] == a[5]);
}

/* HGL_LIFETIME_MANAGED_MALLOC_ALLOCATOR */

TEST(lmm_test_simple_alloc) {
    int *a[10];

    a[0] = hgl_alloc(&lmmalloc, sizeof(int));
    a[1] = hgl_alloc(&lmmalloc, sizeof(int));
    a[2] = hgl_alloc(&lmmalloc, sizeof(int));
    a[3] = hgl_alloc(&lmmalloc, sizeof(int));
    a[4] = hgl_alloc(&lmmalloc, sizeof(int));
    a[5] = hgl_alloc(&lmmalloc, sizeof(int));

    ASSERT(hgl_alloc_usage(&lmmalloc) == 6*sizeof(int));

    a[2] = hgl_realloc(&lmmalloc, a[2], 20 * sizeof(int));
    a[3] = hgl_realloc(&lmmalloc, a[3], 2000000 * sizeof(int));

    ASSERT(hgl_alloc_usage(&lmmalloc) == 2000024*sizeof(int));


    hgl_free(&lmmalloc, a[0]); // -1
    hgl_free(&lmmalloc, a[4]); // -1
    hgl_free(&lmmalloc, a[3]); // -2000000

    ASSERT(hgl_alloc_usage(&lmmalloc) == 22*sizeof(int));

    hgl_free_all(&lmmalloc);

    ASSERT(hgl_alloc_usage(&lmmalloc) == 0);
}
