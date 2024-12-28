
#include "hgl_test.h"

#include "hgl_da.h"

typedef HglDynamicArray(int) DaInt;

static DaInt da = {0};

GLOBAL_TEARDOWN {
    hgl_da_free(&da);
}

TEST(test_push) {
    hgl_da_push(&da,  1);
    hgl_da_push(&da,  2);
    hgl_da_push(&da,  4);
    hgl_da_push(&da,  8);
    hgl_da_push(&da, 16);
    ASSERT(da.length == 5);
    ASSERT(da.capacity >= 5);

    (void) hgl_da_pop(&da);
    ASSERT(da.length == 4);
    ASSERT(da.capacity >= 5);
}

TEST(test_reserve) {
    hgl_da_reserve(&da,  6071);
    ASSERT(da.length == 0);
    ASSERT(da.capacity >= 6071);
}

TEST(test_extend) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    int arr[] = {4,5,6,7,8};
    hgl_da_extend(&da, arr, sizeof(arr)/sizeof(arr[0]));
    ASSERT(da.length == 8);
    ASSERT(da.arr[7] == 8);
    ASSERT(da.arr[3] == 4);
}

TEST(test_extend2) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    int arr[] = {4,5,6,7,8};
    hgl_da_extend(&da, arr, sizeof(arr)/sizeof(arr[0]));
    hgl_da_extend(&da, arr, sizeof(arr)/sizeof(arr[0]));
    ASSERT(da.length == 8 + 5);
    ASSERT(da.arr[7] == 8);
    ASSERT(da.arr[3] == 4);
}

TEST(test_get) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    hgl_da_push(&da, 4);
    hgl_da_push(&da, 5);
    ASSERT(hgl_da_get(&da, -1) == 5);
    ASSERT(hgl_da_get(&da, 0) == 1);
    ASSERT(hgl_da_get(&da, 3) == 4);
}

TEST(test_at) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    hgl_da_push(&da, 4);
    hgl_da_push(&da, 5);
    hgl_da_at(&da, -1) = 16;
    ASSERT(hgl_da_get(&da, -1) == 16);
    ASSERT(hgl_da_get(&da, 0) == 1);
    ASSERT(hgl_da_get(&da, 3) == 4);
}

TEST(test_insert) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    hgl_da_push(&da, 4);
    hgl_da_push(&da, 5);
    hgl_da_insert(&da, 2, 1337);
    ASSERT(da.length == 6);
    ASSERT(da.arr[1] == 2);
    ASSERT(da.arr[2] == 1337);
    ASSERT(da.arr[3] == 3);
}

TEST(test_remove) {
    hgl_da_push(&da, 1);
    hgl_da_push(&da, 2);
    hgl_da_push(&da, 3);
    hgl_da_push(&da, 4);
    hgl_da_push(&da, 5);
    hgl_da_insert(&da, 2, 1337);
    ASSERT(da.length == 6);
    ASSERT(da.arr[1] == 2);
    ASSERT(da.arr[2] == 1337);
    ASSERT(da.arr[3] == 3);
    hgl_da_remove(&da, 2);
    ASSERT(da.arr[1] == 2);
    ASSERT(da.arr[2] == 3);
}
