
#include "hgl_test.h"

#define HGL_ARENA_ALLOC_IMPLEMENTATION
#define HGL_STACK_ALLOC_IMPLEMENTATION
#include "hgl_stack_alloc.h"

static HglArena arena;

GLOBAL_SETUP {
    arena = hgl_arena_make(128 * 1024, HGL_ARENA_MALLOC, NULL); // 128 KiB
}

GLOBAL_TEARDOWN {
    hgl_arena_destroy(&arena);
}

TEST(simple_alloc) {
    void *p0 = hgl_stack_alloc(&arena, 100); 
    void *p1 = hgl_stack_alloc(&arena, 1000); 
    void *p2 = hgl_stack_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_stack_alloc(&arena, 0) == NULL);
}

TEST(test_free_all_behavior) {
    void *p0 = hgl_stack_alloc(&arena, 100); 
    void *p1 = hgl_stack_alloc(&arena, 1000); 
    void *p2 = hgl_stack_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_arena_free_all(&arena);

    void *p3 = hgl_stack_alloc(&arena, 100);

    ASSERT(p0 == p3);
}

TEST(test_too_big_allocation) {
    void *p0 = hgl_stack_alloc(&arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(test_realloc) {
    unsigned int *p0 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    unsigned int *p4 = hgl_stack_realloc(&arena, p3, sizeof(unsigned int));
    ASSERT(p4 != NULL);
    ASSERT(p4 == p3);
}

TEST(test_realloc_invalid_ptr, .expect_signal = SIGABRT) {
    unsigned int *p0 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_stack_alloc(&arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    p3 = hgl_stack_realloc(&arena, p1, sizeof(unsigned int));
}

TEST(test_free_last) {
    void *p0 = hgl_stack_alloc(&arena, 100); 
    void *p1 = hgl_stack_alloc(&arena, 1000); 
    void *p2 = hgl_stack_alloc(&arena, 10000); 

    hgl_stack_free(&arena, p2);

    void *p3 = hgl_stack_alloc(&arena, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}

TEST(test_free_non_last, .expect_signal = SIGABRT) {
    void *p0 = hgl_stack_alloc(&arena, 100); 
    void *p1 = hgl_stack_alloc(&arena, 1000); 
    void *p2 = hgl_stack_alloc(&arena, 10000); 

    hgl_stack_free(&arena, p1);

    void *p3 = hgl_stack_alloc(&arena, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}
