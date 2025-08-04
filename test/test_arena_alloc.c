
#include "hgl_test.h"

#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

static HglArena bump_arena;
static HglArena stack_arena;

GLOBAL_SETUP {
    bump_arena  = hgl_arena_make(.size = 128 * 1024, .alignment = 1024); // 128 KiB
    stack_arena = hgl_arena_make(.size = 128 * 1024, .alignment = 1024, .kind = HGL_ARENA_STACK_ALLOCATOR); // 128 KiB
}

GLOBAL_TEARDOWN {
    hgl_arena_destroy(&bump_arena);
    hgl_arena_destroy(&stack_arena);
}

TEST(bump_test_simple_alloc) {
    void *p0 = hgl_arena_alloc(&bump_arena, 100); 
    void *p1 = hgl_arena_alloc(&bump_arena, 1000); 
    void *p2 = hgl_arena_alloc(&bump_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_arena_alloc(&bump_arena, 0) == NULL);
}

TEST(bump_test_free_all_behavior) {
    void *p0 = hgl_arena_alloc(&bump_arena, 100); 
    void *p1 = hgl_arena_alloc(&bump_arena, 1000); 
    void *p2 = hgl_arena_alloc(&bump_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_arena_free_all(&bump_arena);

    void *p3 = hgl_arena_alloc(&bump_arena, 100);

    ASSERT(p0 == p3);
}

TEST(bump_test_too_big_allocation) {
    void *p0 = hgl_arena_alloc(&bump_arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(bump_test_alignment) {
    void *p0 = hgl_arena_alloc(&bump_arena, 1); 
    void *p1 = hgl_arena_alloc(&bump_arena, 1); 
    void *p2 = hgl_arena_alloc(&bump_arena, 1); 

    ASSERT(p1 - p0 == bump_arena.config.alignment);
    ASSERT(p2 - p1 == bump_arena.config.alignment);
}

TEST(test_realloc) {
    void *p0 = hgl_arena_alloc(&stack_arena, 10); 
    void *p1 = hgl_arena_alloc(&stack_arena, 20); 

    size_t previous_arena_head = stack_arena.head;
    void *p2 = hgl_arena_realloc(&stack_arena, p1, 20);
    size_t new_arena_head = stack_arena.head;

    ASSERT(previous_arena_head == new_arena_head);
}

TEST(
    test_realloc_invalid_pointer, 
    .expect_signal = SIGABRT
) {
    void *p0 = hgl_arena_alloc(&stack_arena, 10); 
    void *p1 = hgl_arena_alloc(&stack_arena, 20); 
    void *p2 = hgl_arena_realloc(&stack_arena, p0, 20);
}

TEST(stack_simple_alloc) {
    void *p0 = hgl_arena_alloc(&stack_arena, 100); 
    void *p1 = hgl_arena_alloc(&stack_arena, 1000); 
    void *p2 = hgl_arena_alloc(&stack_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_arena_alloc(&stack_arena, 0) == NULL);
}

TEST(stack_free_all_behavior) {
    void *p0 = hgl_arena_alloc(&stack_arena, 100); 
    void *p1 = hgl_arena_alloc(&stack_arena, 1000); 
    void *p2 = hgl_arena_alloc(&stack_arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_arena_free_all(&stack_arena);

    void *p3 = hgl_arena_alloc(&stack_arena, 100);

    ASSERT(p0 == p3);
}

TEST(stack_too_big_allocation) {
    void *p0 = hgl_arena_alloc(&stack_arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(stack_realloc) {
    unsigned int *p0 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    unsigned int *p4 = hgl_arena_realloc(&stack_arena, p3, sizeof(unsigned int));
    ASSERT(p4 != NULL);
    ASSERT(p4 == p3);
}

TEST(stack_realloc_invalid_ptr, .expect_signal = SIGABRT) {
    unsigned int *p0 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p1 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p2 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    unsigned int *p3 = hgl_arena_alloc(&stack_arena, sizeof(unsigned int)); 
    *p0 = 0x11111111;
    *p1 = 0x22222222;
    *p2 = 0x33333333;
    *p3 = 0x44444444;
    p3 = hgl_arena_realloc(&stack_arena, p1, sizeof(unsigned int));
}

TEST(stack_free_last) {
    void *p0 = hgl_arena_alloc(&stack_arena, 100); 
    void *p1 = hgl_arena_alloc(&stack_arena, 1000); 
    void *p2 = hgl_arena_alloc(&stack_arena, 10000); 

    hgl_arena_free(&stack_arena, p2);

    void *p3 = hgl_arena_alloc(&stack_arena, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}

TEST(stack_free_non_last, .expect_signal = SIGABRT) {
    void *p0 = hgl_arena_alloc(&stack_arena, 100); 
    void *p1 = hgl_arena_alloc(&stack_arena, 1000); 
    void *p2 = hgl_arena_alloc(&stack_arena, 10000); 

    hgl_arena_free(&stack_arena, p1);

    void *p3 = hgl_arena_alloc(&stack_arena, 100);

    ASSERT(p3 != NULL);
    ASSERT(p2 == p3);
}
