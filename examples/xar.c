
#define HGL_XAR_IMPLEMENTATION
#include "hgl_xar.h"

#include <stdio.h>
#include <stdlib.h>


static inline void *my_alloc(size_t size)
{
    return aligned_alloc(256, size);
}

typedef HglXar(int) Ints;

int main(void)
{
    printf("Hello World!\n");
    Ints ints = hgl_xar_make(int, .shift = MSB32(4096));

    printf("#1\n");
    for (int i = 0; i < 100000000; i++) {
        hgl_xar_push(&ints, i);
    }

    for (size_t i = 0; i < ints.header.count; i++) {
        //int *el = hgl_xar_pop(&ints);
        //printf("popped: %d\n", *el);
        int *el = hgl_xar_get(&ints, i);
        if (i == ints.header.count - 1)
            printf("got: %d\n", *el);
    }

    hgl_xar_clear(&ints);

    printf("#2\n");

    hgl_xar_push(&ints, 1);
    printf("a\n");
    hgl_xar_push(&ints, 2);
    printf("b\n");
    hgl_xar_push(&ints, 4);
    printf("c\n");
    hgl_xar_push(&ints, 8);
    printf("d\n");
    hgl_xar_push(&ints, 16);
    printf("e\n");

    hgl_xar_remove_backswap(&ints, 0);

    printf("f\n");

    for (size_t i = 0; i < hgl_xar_count(&ints); i++) {
        int *el = hgl_xar_get(&ints, i);
        printf("got: %d\n", *el);
    }

    hgl_xar_destroy(&ints);

    printf("#3\n");
    ints = (Ints) hgl_xar_make(int, .shift = MSB32(16));
    for (int i = 0; i < 1000; i++) {
        hgl_xar_push(&ints, i);
    }

    int *buf = malloc(hgl_xar_count(&ints)*sizeof(int));
    size_t n_bytes = hgl_xar_copy_to_array(&ints, buf);

    for (size_t i = 0; i < hgl_xar_count(&ints); i++) {
        printf("%d\n", buf[i]);
    }
    printf("%zu\n", n_bytes);
    n_bytes = hgl_xar_copy_first_n_to_array(&ints, NULL, 10);
    printf("%zu\n", n_bytes);
    free(buf);
    hgl_xar_clear(&ints);

    printf("#3\n");
    for (int i = 0; i < 32; i++) {
        hgl_xar_push(&ints, i);
    }
    int *arr = hgl_xar_to_array(&ints);
    size_t count = hgl_xar_count(&ints);
    for (size_t i = 0; i < count; i++) {
        printf("arr[%zu] = %d\n", i, arr[i]);
    }
    free(arr);
    hgl_xar_destroy(&ints);
}
