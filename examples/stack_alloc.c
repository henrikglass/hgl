#include <stdio.h>

#define HGL_ARENA_ALIGNMENT 32
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#define HGL_STACK_ALLOC_IMPLEMENTATION
#include "hgl_stack_alloc.h"

static uint8_t memory_chunk[128 * 1024];

static HglArena s_arena;

void *arena_alloc(size_t size)
{
    return hgl_arena_alloc(&s_arena, size);
}

void *stack_alloc(size_t size)
{
    return hgl_stack_alloc(&s_arena, size);
}

void stack_free(void *ptr)
{
    hgl_stack_free(&s_arena, ptr);
}

int main()
{
    //s_arena = hgl_arena_make(256 * 1024);  // 128 KiB
    s_arena = hgl_arena_make(sizeof(memory_chunk), HGL_ARENA_STATIC, memory_chunk);

    void *arena_alloced_thing = arena_alloc(32);

    void *a = stack_alloc(32);
    void *b = stack_alloc(128);
    void *c = stack_alloc(512);
    stack_free(c);
    stack_free(b);
    //stack_free(a);
    void *d = stack_alloc(4096);

    printf("%p\n", arena_alloced_thing);
    printf("%p\n", a);
    printf("%p\n", b);
    printf("%p\n", c);
    printf("%p\n", d);

    printf("%08lX\n", (uint8_t*)arena_alloced_thing - s_arena.memory);
    printf("%08lX\n", (uint8_t*)a - s_arena.memory);
    printf("%08lX\n", (uint8_t*)b - s_arena.memory);
    printf("%08lX\n", (uint8_t*)c - s_arena.memory);
    printf("%08lX\n", (uint8_t*)d - s_arena.memory);

    hgl_arena_free_all(&s_arena);

    a = stack_alloc(128 * 1024);
    printf("%08lX\n", (uint8_t*)a - s_arena.memory);

    //hgl_arena_destroy(&s_arena);
}
