#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "hgl.h"
#include "hgl_da.h"

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

//typedef struct {
    //float *arr;
    //size_t length;
    //size_t capacity;
//} Floats;

typedef HglDynamicArray(float) Floats;

typedef struct {
    int a, b;
} MyType;

int test() {
    try(fcntl(2124, 124, 0)); // TRY something that will fail. Should return -1.
    return 0;
}

int main()
{
    hgl_show_build_info();
    f32 fisk = 123.0f;
    printf("fisk = %f\n", (double)fisk);
    printf("fisk bits = 0x%08X\n", transmute(u32, fisk));
    printf("fisk = %f\n", (double)transmute(float, transmute(u32, fisk)));
    assert(test() == -1);

    HglDynamicArray(float) fs = {0};
    hgl_da_reserve(&fs, 1024);
    float ffs[5] = {1.1f, 1.2f, 1.3f, 1.4f, 1.5f};
    hgl_da_push(&fs, 1.0f);
    hgl_da_push(&fs, 1.05f);
    hgl_da_extend(&fs, ffs, sizeof(ffs)/sizeof(ffs[0]));
    printf("len = %ld\n", fs.length);
    hgl_da_insert(&fs, fs.length - 1, 123.456f);
    hgl_da_remove(&fs, 1);
    hgl_da_insert(&fs, 1, 1.055f);
    hgl_da_insert(&fs, 1, 1.054f);
    hgl_da_insert(&fs, 1, 1.053f);
    hgl_da_insert(&fs, 1, 1.052f);
    hgl_da_insert(&fs, 0, 0.95f);

    for (size_t i = 0; i < fs.length; i++) {
        printf("%f\n", fs.arr[i]);
    }
    printf("cap = %zu\n", fs.capacity);
    hgl_da_free(&fs);

    printf("last = %f\n", hgl_da_remove(&fs, fs.length - 1));
    printf("last = %f\n", hgl_da_pop(&fs));
    printf("fist = %f\n", hgl_da_remove(&fs, 0));

    for (size_t i = 0; i < fs.length; i++) {
        printf("%f\n", fs.arr[i]);
    }
    printf("cap = %zu\n", fs.capacity);

    MyType a = {1, 2};
    MyType b = {3, 4};
    HglDynamicArray(MyType) da = {0};
    hgl_da_push(&da, a);
    hgl_da_push(&da, b);
    hgl_da_free(&da);

    hgl_memdbg_report();

    return 0;
}
