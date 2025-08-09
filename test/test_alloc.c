
#include "hgl_test.h"

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"

static HglAllocator bump_arena;
static HglAllocator stack_arena;
static HglAllocator fs_allocator;
static HglAllocator pool;

GLOBAL_SETUP {
    bump_arena = hgl_alloc_make(.kind = HGL_BUMP_ARENA,  
                                .size = 128 * 1024, 
                                .alignment = 1024); // 128 KiB
    stack_arena = hgl_alloc_make(.kind = HGL_STACK_ARENA, 
                                 .size = 128 * 1024, 
                                 .alignment = 1024); // 128 KiB
    fs_allocator = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR, 
                                  .size = 64*1024, 
                                  .free_stack_capacity = 16, 
                                  .alignment = 128);
    pool = hgl_alloc_make(.kind = HGL_POOL_ALLOCATOR, 
                          .size = 16*50, 
                          .pool_chunk_size = 50);
}

GLOBAL_TEARDOWN {
    hgl_alloc_destroy(&bump_arena);
    hgl_alloc_destroy(&stack_arena);
    hgl_alloc_destroy(&fs_allocator);
    hgl_alloc_destroy(&pool);
}

/* HGL_BUMP_ARENA */

TEST(bump_test_simple_alloc) {
    void *p0 = hgl_alloc(&bump_arena, 100); 
    void *p1 = hgl_alloc(&bump_arena, 1000); 
    void *p2 = hgl_alloc(&bump_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_alloc(&bump_arena, 0) == NULL);
}

TEST(bump_test_free_all_behavior) {
    void *p0 = hgl_alloc(&bump_arena, 100); 
    void *p1 = hgl_alloc(&bump_arena, 1000); 
    void *p2 = hgl_alloc(&bump_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&bump_arena);

    void *p3 = hgl_alloc(&bump_arena, 100);

    ASSERT(p0 == p3);
}

TEST(bump_test_too_big_allocation) {
    void *p0 = hgl_alloc(&bump_arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(bump_test_alignment) {
    void *p0 = hgl_alloc(&bump_arena, 1); 
    void *p1 = hgl_alloc(&bump_arena, 1); 
    void *p2 = hgl_alloc(&bump_arena, 1); 

    ASSERT(p1 - p0 == bump_arena.config.alignment);
    ASSERT(p2 - p1 == bump_arena.config.alignment);
}

/* HGL_STACK_ARENA */

TEST(stack_test_realloc) {
    void *p0 = hgl_alloc(&stack_arena, 10); 
    void *p1 = hgl_alloc(&stack_arena, 20); 

    size_t previous_arena_head = stack_arena.head;
    void *p2 = hgl_realloc(&stack_arena, p1, 20);
    size_t new_arena_head = stack_arena.head;

    ASSERT(previous_arena_head == new_arena_head);
}

TEST(
    stack_test_realloc_invalid_pointer, 
    .expect_signal = SIGABRT
) {
    void *p0 = hgl_alloc(&stack_arena, 10); 
    void *p1 = hgl_alloc(&stack_arena, 20); 
    void *p2 = hgl_realloc(&stack_arena, p0, 20);
}

TEST(stack_simple_alloc) {
    void *p0 = hgl_alloc(&stack_arena, 100); 
    void *p1 = hgl_alloc(&stack_arena, 1000); 
    void *p2 = hgl_alloc(&stack_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_alloc(&stack_arena, 0) == NULL);
}

TEST(stack_free_all_behavior) {
    void *p0 = hgl_alloc(&stack_arena, 100); 
    void *p1 = hgl_alloc(&stack_arena, 1000); 
    void *p2 = hgl_alloc(&stack_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_free_all(&stack_arena);

    void *p3 = hgl_alloc(&stack_arena, 100);

    ASSERT(p0 == p3);
}

TEST(stack_too_big_allocation) {
    void *p0 = hgl_alloc(&stack_arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(stack_realloc) {
    unsigned int *p0 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    unsigned int *p4 = hgl_realloc(&stack_arena, p3, sizeof(unsigned int));
    ASSERT(p4 != NULL);
    ASSERT(p4 == p3);
}

TEST(stack_realloc_invalid_ptr, .expect_signal = SIGABRT) {
    unsigned int *p0 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_alloc(&stack_arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    p3 = hgl_realloc(&stack_arena, p1, sizeof(unsigned int));
}

TEST(stack_free_last) {
    void *p0 = hgl_alloc(&stack_arena, 100); 
    void *p1 = hgl_alloc(&stack_arena, 1000); 
    void *p2 = hgl_alloc(&stack_arena, 10000); 

    hgl_free(&stack_arena, p2);

    void *p3 = hgl_alloc(&stack_arena, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}

TEST(stack_free_non_last, .expect_signal = SIGABRT) {
    void *p0 = hgl_alloc(&stack_arena, 100); 
    void *p1 = hgl_alloc(&stack_arena, 1000); 
    void *p2 = hgl_alloc(&stack_arena, 10000); 

    hgl_free(&stack_arena, p1);

    void *p3 = hgl_alloc(&stack_arena, 100);

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
