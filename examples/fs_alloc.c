
#define HGL_FS_ALLOC_ALIGNMENT 16
#define HGL_FS_ALLOC_IMPLEMENTATION
#include "hgl_fs_alloc.h"

#include <stdio.h>
#include <stdbool.h>

static HglFsAllocator allocator;

void *my_alloc(size_t size);
void my_free(void *ptr);
void my_free_all(void);

void *my_alloc(size_t size) {
    void *ptr = hgl_fs_alloc(&allocator, size);
    printf("0x%lX\n", (uint8_t *)ptr - allocator.memory);
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    void *retptr = hgl_fs_realloc(&allocator, ptr, size);
    printf("0x%lX\n", (uint8_t *)retptr - allocator.memory);
    return retptr;
}

void my_free(void *ptr) {
    printf("FREE\n");
    hgl_fs_free(&allocator, ptr);
}

void my_free_all(void) {
    hgl_fs_free_all(&allocator);
}

#define PRINT_EVERYTHING \
    do {\
        printf("\n"); \
        printf("======================================================================\n"); \
        memset(vis, 'X', 0x40); \
        for (int idx = 0; idx < allocator.free_count; idx++) { \
            printf("{0x%lX, 0x%lX} ", allocator.free_stack[idx].start - allocator.memory, \
                                      allocator.free_stack[idx].end - allocator.memory); \
            memset(vis + ((allocator.free_stack[idx].start - allocator.memory) >> 4), '_', \
                   (allocator.free_stack[idx].end - allocator.free_stack[idx].start) >> 4); \
        } \
        printf("\n"); \
        printf("%s\n", vis); \
        printf("======================================================================\n"); \
        printf("\n"); \
    } while (0)

static uint8_t buffer[2048];

int main(void)
{
    printf("Hello World!\n");

    bool malloced = false;

#if 0
    allocator = hgl_fs_make(2048, 8);
    malloced = true;
#else
    allocator = hgl_fs_make_from_buffer((void *) buffer, 2048, 4);
#endif

    char vis[0x81];
    vis[0x80] = '\0';

#if 1
    int *a = my_alloc(1);
    int *b = my_alloc(4);
    int *c = my_alloc(16);
    my_alloc(16);
    PRINT_EVERYTHING;
    
    int *k = my_alloc(0x80);
    PRINT_EVERYTHING;
    my_free(a);
    my_free(b);
    my_free(c);
    PRINT_EVERYTHING;

    int *d = my_alloc(0x20);
    PRINT_EVERYTHING;
    int *d2 = my_alloc(0x20);
    PRINT_EVERYTHING;
    int *d3 = my_alloc(0x20);
    PRINT_EVERYTHING;

    my_alloc(0x200);
    PRINT_EVERYTHING;
    my_alloc(0x80);
    PRINT_EVERYTHING;
    my_alloc(0x80);
    PRINT_EVERYTHING;

    my_free(d);
    my_free(d2);
    my_free(d3);
    //my_free(k);

    my_alloc(0x80);
    PRINT_EVERYTHING;

    my_free(k);
    PRINT_EVERYTHING;

    int * asd = my_alloc(0x80);
    PRINT_EVERYTHING;
    
    my_realloc(asd, 0x80);
    PRINT_EVERYTHING;
#endif

#if 0
    PRINT_EVERYTHING;

    int *a = my_alloc(24);
    int *b = my_alloc(24);
    int *c = my_alloc(24);
    int *d = my_alloc(128);
    int *e = my_alloc(4);
    int *f = my_alloc(4);

    printf("%lX\n", (uint8_t *)a - allocator.memory);
    printf("%lX\n", (uint8_t *)b - allocator.memory);
    printf("%lX\n", (uint8_t *)c - allocator.memory);
    printf("%lX\n", (uint8_t *)d - allocator.memory);
    printf("%lX\n", (uint8_t *)e - allocator.memory);
    printf("%lX\n", (uint8_t *)f - allocator.memory);


    PRINT_EVERYTHING;


    my_free(d);

    PRINT_EVERYTHING;

    int *g = my_alloc(4);
    int *h = my_alloc(4);

    printf("%lX\n", (uint8_t *)a - allocator.memory);
    printf("%lX\n", (uint8_t *)b - allocator.memory);
    printf("%lX\n", (uint8_t *)c - allocator.memory);
    printf("%lX\n", (uint8_t *)d - allocator.memory);
    printf("%lX\n", (uint8_t *)e - allocator.memory);
    printf("%lX\n", (uint8_t *)f - allocator.memory);
    printf("%lX\n", (uint8_t *)g - allocator.memory);
    printf("%lX\n", (uint8_t *)h - allocator.memory);

    PRINT_EVERYTHING;

    int *i = my_alloc(0x40);
    int *j = my_alloc(0x40);
    printf("%lX\n", (uint8_t *)i - allocator.memory);
    printf("%lX\n", (uint8_t *)j - allocator.memory);

    PRINT_EVERYTHING;

    my_free(c);

    PRINT_EVERYTHING;

    my_free(b);

    PRINT_EVERYTHING;

    int *k = my_alloc(8);
    printf("%lX\n", (uint8_t *)k - allocator.memory);

    PRINT_EVERYTHING;

    my_free_all();

    PRINT_EVERYTHING;

#endif

    if (malloced) {
        hgl_fs_destroy(&allocator);
    }

    printf("END\n");

    return 0;
}
