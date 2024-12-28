
#include "hgl_test.h"

#define HGL_MEMDBG_IMPLEMENTATION
#include "hgl_memdbg.h"

TEST(test_no_leak) {
    void *p0 = malloc(128);
    free(p0);

    ASSERT(hgl_memdbg_report() == 0);
}

TEST(test_leak) {
    void *p0 = malloc(128);

    ASSERT(hgl_memdbg_report() == -1);
}

TEST(test_example) {
    double *d = malloc(sizeof(double)); 
    double *arr = malloc(64 * sizeof(double)); 
    arr[0] = 1.0f;
    arr[1] = 2.0f;
    arr[2] = 3.0f;
    arr[3] = 4.0f;
    double *arr2 = malloc(64 * sizeof(double)); 
    *d = 123.456;

    printf("Hello world! %f\n", *d);
    double *arr3 = malloc(64 * sizeof(double)); 
    free(arr2);
    printf("%f\n", arr[2]);
    arr = realloc(arr, 128*sizeof(double));
    printf("%f\n", arr[2]);
    free(arr);
    free(d);

    ASSERT(hgl_memdbg_report() == -1);
}

TEST(test_realloc) {
    void *p0 = malloc(16);
    void *p1 = malloc(32);
    void *p2 = malloc(64);

    p1 = realloc(p1, 64);

    free(p0);
    free(p1);
    free(p2);

    ASSERT(hgl_memdbg_report() == 0);
}

TEST(test_alloc_zero) {
    void *p0 = malloc(0);
    ASSERT(p0 == NULL);
    ASSERT(hgl_memdbg_report() == 0);
}

TEST(test_free_null) {
    free(NULL);
    ASSERT(hgl_memdbg_report() == 0);
}
