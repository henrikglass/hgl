#include <stdio.h>

#define HGL_ARENA_ALIGNMENT 64
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

#if 0
static HglArena s_arena;
#else
static HglArena s_arena = HGL_ARENA_INITIALIZER(HGL_ARENA_STACK_ALLOCATOR, 128 * 1024);
#endif

void *arena_alloc(size_t size)
{
    return hgl_arena_alloc(&s_arena, size);
}

int main()
{

#if 0
    s_arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR);
#endif
#if 0
    s_arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_BUMP_ALLOCATOR);
#endif
#if 0
    s_arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR, .backend = HGL_ARENA_MMAP_HUGEPAGE_2MB);
#endif
#if 0
    s_arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR, .alignment = 1024);
#endif


    void *a = arena_alloc(32);
    void *b = arena_alloc(128);
    void *c = arena_alloc(512);
    void *d = arena_alloc(4096);

    printf("%p\n", a);
    printf("%p\n", b);
    printf("%p\n", c);
    printf("%p\n", d);

    printf("%08lX\n", (uint8_t*)a - s_arena.memory);
    printf("%08lX\n", (uint8_t*)b - s_arena.memory);
    printf("%08lX\n", (uint8_t*)c - s_arena.memory);
    printf("%08lX\n", (uint8_t*)d - s_arena.memory);

    /* these two are equivalent */
#if 0
    hgl_arena_free(&s_arena, d);
#else
    hgl_arena_pop(&s_arena);
#endif

    hgl_arena_free_all(&s_arena);

    a = arena_alloc(128 * 1024);

    printf("%08lX\n", (uint8_t*)a - s_arena.memory);

    //hgl_arena_destroy(&s_arena);
}
