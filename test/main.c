#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

//#define HGL_VEC_TYPE int
//#define HGL_VEC_TYPE_ID int
//#include "hgl_vec.h"
//
//#undef HGL_VEC_TYPE
//#undef HGL_VEC_TYPE_ID
//#define HGL_VEC_TYPE float
//#define HGL_VEC_TYPE_ID float
//#include "hgl_vec.h"

#define HGL_RBUF_TYPE float
#define HGL_RBUF_TYPE_ID float
#include "hgl_ringbuf.h"

#define STR(x)   #x
#define SHOW_DEFINE(x) printf("%s=%s\n", #x, STR(x))

int main(void)
{
    printf("Hello world!\n");

#if 1
    SHOW_DEFINE(HGL_VEC_TYPE);
    SHOW_DEFINE(HGL_VEC_TYPE_PREFIX);
    SHOW_DEFINE(HGL_VEC_TYPE_SIZE);
    SHOW_DEFINE(HEADER_GUARD);
    SHOW_DEFINE(HGL_VEC_STRUCT_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_INIT_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_INIT_CAPACITY_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_FREE_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_RESERVE_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_PUSH_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_POP_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_REMOVE_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_EXTEND_NAME);
    SHOW_DEFINE(HGL_VEC_FUNC_EXTEND_ARRAY_NAME);
    SHOW_DEFINE(HGL_VEC_GROWTH_TYPE);
    SHOW_DEFINE(EXPONENTIAL);
    SHOW_DEFINE(HGL_VEC_LINEAR_GROWTH_RATE);
    SHOW_DEFINE(HGL_VEC_EXPONENTIAL_GROWTH_RATE);
#endif

    //hgl_int_vec_t v;
    //hgl_int_vec_t v2;
    //hgl_float_vec_t vf;
    hgl_float_rbuf_t rbuf;
    
    float *f;
    hgl_float_rbuf_init(&rbuf, 8);
    hgl_float_rbuf_push_back(&rbuf, 1.0f);
    hgl_float_rbuf_push_back(&rbuf, 2.0f);
    hgl_float_rbuf_push_back(&rbuf, 3.0f);
    hgl_float_rbuf_push_back(&rbuf, 6.0f);
    f = hgl_float_rbuf_pop_front(&rbuf);
    f = hgl_float_rbuf_pop_front(&rbuf);
    f = hgl_float_rbuf_pop_front(&rbuf);
    f = hgl_float_rbuf_pop_front(&rbuf);
    f = hgl_float_rbuf_pop_front(&rbuf);
    f = hgl_float_rbuf_pop_front(&rbuf);
    hgl_float_rbuf_push_back(&rbuf, 1.0f);
    hgl_float_rbuf_push_back(&rbuf, 1.0f);
    hgl_float_rbuf_push_back(&rbuf, 1.0f);
    hgl_float_rbuf_push_back(&rbuf, 2.0f);
    hgl_float_rbuf_push_back(&rbuf, 3.0f);
    hgl_float_rbuf_push_back(&rbuf, 6.0f);
    hgl_float_rbuf_push_back(&rbuf, 2.0f);
    hgl_float_rbuf_push_back(&rbuf, 3.0f);
    hgl_float_rbuf_push_back(&rbuf, 6.0f);
    hgl_float_rbuf_push_back(&rbuf, 2.0f);
    hgl_float_rbuf_push_back(&rbuf, 3.0f);
    hgl_float_rbuf_push_back(&rbuf, 6.0f);
    f = hgl_float_rbuf_pop_front(&rbuf);
    hgl_float_rbuf_free(&rbuf);
    (void) f;

#if 0
    hgl_float_vec_init(&vf);
    hgl_float_vec_push(&vf, 23.4f);
    hgl_float_vec_push(&vf, 20.4f);
    hgl_float_vec_push(&vf, 8.4f);
    hgl_float_vec_push(&vf, 27.4f);
    hgl_float_vec_pop(&vf);
    hgl_float_vec_free(&vf);
#endif

#if 0
    int arr[] = {4, 2, 0};
    hgl_int_vec_init_capacity(&v, 2);
    hgl_int_vec_init(&v2);
    hgl_int_vec_extend_array(&v, arr, 3);
    hgl_int_vec_shrink_to_fit(&v);
    hgl_int_vec_push(&v, 1);
    hgl_int_vec_push(&v, 3);
    hgl_int_vec_push(&v, 3);
    hgl_int_vec_push(&v, 7);
    hgl_int_vec_push(&v2, 4);
    hgl_int_vec_push(&v2, 2);
    hgl_int_vec_push(&v2, 0);
    hgl_int_vec_push(&v2, 6);
    hgl_int_vec_push(&v2, 9);
    
    hgl_int_vec_insert(&v, &v2, 2);

    hgl_int_vec_free(&v);
    hgl_int_vec_free(&v2);
#endif
#if 0
    hgl_int_vec_init(&v);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vec_push(&v, 1);
    hgl_int_vec_push(&v, 3);
    hgl_int_vec_push(&v, 3);
    hgl_int_vec_push(&v, 7);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);

    hgl_int_vec_init(&v2);
    hgl_int_vec_push(&v2, 4);
    hgl_int_vec_push(&v2, 2);
    hgl_int_vec_push(&v2, 0);

    hgl_int_vec_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vec_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vec_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vec_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);

    hgl_int_vec_pop(&v);
    hgl_int_vec_pop(&v);
    int four = hgl_int_vec_pop(&v);
    printf("four: %d\n", four);

    for (size_t i = 0; i < v.len; i++) {
        printf("%d ", v.arr[i]);
    }
    printf("\n");
   
    hgl_int_vec_remove(&v, 0);
    hgl_int_vec_sort(&v, NULL);
    int seven = 7;
    int *resp = hgl_int_vec_bsearch(&v, &seven, NULL);

    printf("res: %p\n", (void*)resp);
    printf("arrbegin: %p\n", (void*)v.arr);

    for (size_t i = 0; i < v.len; i++) {
        printf("%d ", v.arr[i]);
    }
    printf("\n");

    printf("kalas\n");
    
    printf("v  arr: %p\n", (void *) v.arr);

    hgl_int_vec_free(&v);
    
    printf("v2 arr: %p\n", (void *) v2.arr);
    hgl_int_vec_free(&v2);
#endif

    //voidp_vector v;
    //printf("v size: %lu\n"v.size);
}
