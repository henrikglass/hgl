#include <stdio.h>

#define HGL_ARENA_ALIGNMENT 64
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

static uint8_t memory_chunk[128 * 1024];

static HglArena s_arena;

void *arena_alloc(size_t size)
{
    return hgl_arena_alloc(&s_arena, size);
}

int main()
{
    //s_arena = hgl_arena_make(128 * 1024);  // 128 KiB
    s_arena = hgl_arena_make(sizeof(memory_chunk), HGL_ARENA_STATIC, memory_chunk);
    //s_arena = hgl_arena_make(1024);  // 1 KiB


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

    hgl_arena_free_all(&s_arena);

    a = arena_alloc(128 * 1024);

    printf("%08lX\n", (uint8_t*)a - s_arena.memory);

    //hgl_arena_destroy(&s_arena);
}
