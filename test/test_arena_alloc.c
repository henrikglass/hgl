
#include "hgl_test.h"

#define HGL_ARENA_ALIGNMENT 1024
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

static HglArena arena;

GLOBAL_SETUP {
    arena = hgl_arena_make(128 * 1024); // 128 KiB
}

GLOBAL_TEARDOWN {
    hgl_arena_destroy(&arena);
}

TEST(test_simple_alloc) {
    void *p0 = hgl_arena_alloc(&arena, 100); 
    void *p1 = hgl_arena_alloc(&arena, 1000); 
    void *p2 = hgl_arena_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ASSERT(hgl_arena_alloc(&arena, 0) == NULL);
}

TEST(test_free_all_behavior) {
    void *p0 = hgl_arena_alloc(&arena, 100); 
    void *p1 = hgl_arena_alloc(&arena, 1000); 
    void *p2 = hgl_arena_alloc(&arena, 10000); 

    ASSERT(p0 != NULL);
    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    hgl_arena_free_all(&arena);

    void *p3 = hgl_arena_alloc(&arena, 100);

    ASSERT(p0 == p3);
}

TEST(test_too_big_allocation) {
    void *p0 = hgl_arena_alloc(&arena, 128*1024 + 4); 

    ASSERT(p0 == NULL);
}

TEST(test_alignment) {
    void *p0 = hgl_arena_alloc(&arena, 1); 
    void *p1 = hgl_arena_alloc(&arena, 1); 

    ASSERT(p1 - p0 == HGL_ARENA_ALIGNMENT);
}

TEST(test_realloc) {
    void *p0 = hgl_arena_alloc(&arena, 10); 
    void *p1 = hgl_arena_alloc(&arena, 20); 

    size_t previous_arena_head = arena.head;
    void *p2 = hgl_arena_realloc(&arena, p1, 20);
    size_t new_arena_head = arena.head;

    ASSERT(previous_arena_head == new_arena_head);
}

TEST(test_realloc_invalid_pointer, .expect_signal = SIGABRT) {
    void *p0 = hgl_arena_alloc(&arena, 10); 
    void *p1 = hgl_arena_alloc(&arena, 20); 

    void *p2 = hgl_arena_realloc(&arena, p0, 20);
}
