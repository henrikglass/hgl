#include <stdio.h>

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"

#if 0
static HglAllocator arena;
#else
static HglAllocator arena = HGL_ALLOC_STACK_ARENA_INITIALIZER(128 * 1024);
#endif

void *my_alloc(size_t size)
{
    return hgl_alloc(&arena, size);
}

int main()
{

#if 1
    //arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR);
    //arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_BUMP_ALLOCATOR);
    //arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR, .backend = HGL_ARENA_MMAP_HUGEPAGE_2MB);
    //arena = hgl_arena_make(.size = sizeof(memory_chunk), .kind = HGL_ARENA_STACK_ALLOCATOR, .alignment = 1024);


    void *a = my_alloc(32);
    void *b = my_alloc(128);
    void *c = my_alloc(512);
    void *d = my_alloc(4096);

    printf("%p\n", a);
    printf("%p\n", b);
    printf("%p\n", c);
    printf("%p\n", d);

    printf("%08lX\n", (uint8_t*)a - arena.memory);
    printf("%08lX\n", (uint8_t*)b - arena.memory);
    printf("%08lX\n", (uint8_t*)c - arena.memory);
    printf("%08lX\n", (uint8_t*)d - arena.memory);

    hgl_free(&arena, d);

    hgl_free_all(&arena);

    a = my_alloc(128 * 1024);

    printf("%08lX\n", (uint8_t*)a - arena.memory);
    hgl_alloc_destroy(&arena);
#endif

#if 0
    HglAllocator fs_allocator = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR,
                                               .size = 4096,
                                               .free_stack_capacity = 8);
    // TODO example
#endif

#if 0
    HglAllocator pool_allocator = hgl_alloc_make(.kind = HGL_POOL_ALLOCATOR,
                                                 .size = 4096,
                                                 .pool_chunk_size = sizeof(int));
    // TODO example
#endif
}
