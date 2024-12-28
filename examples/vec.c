#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HGL_VECTOR_TYPE int
#define HGL_VECTOR_TYPE_ID int
#include "hgl_vector.h"

#undef HGL_VECTOR_TYPE
#undef HGL_VECTOR_TYPE_ID
#define HGL_VECTOR_TYPE float
#define HGL_VECTOR_TYPE_ID float
#include "hgl_vector.h"

#define STR(x)   #x
#define SHOW_DEFINE(x) printf("%s=%s\n", #x, STR(x))

int main(void)
{
    printf("Hello world!\n");

#if 1
    SHOW_DEFINE(HGL_VECTOR_TYPE);
    SHOW_DEFINE(HGL_VECTOR_TYPE_PREFIX);
    SHOW_DEFINE(HGL_VECTOR_TYPE_SIZE);
    SHOW_DEFINE(HEADER_GUARD);
    SHOW_DEFINE(HGL_VECTOR_STRUCT);
    SHOW_DEFINE(HGL_VECTOR_FUNC_INIT);
    SHOW_DEFINE(HGL_VECTOR_FUNC_INIT_CAPACITY);
    SHOW_DEFINE(HGL_VECTOR_FUNC_FREE);
    SHOW_DEFINE(HGL_VECTOR_FUNC_RESERVE);
    SHOW_DEFINE(HGL_VECTOR_FUNC_PUSH);
    SHOW_DEFINE(HGL_VECTOR_FUNC_POP);
    SHOW_DEFINE(HGL_VECTOR_FUNC_REMOVE);
    SHOW_DEFINE(HGL_VECTOR_FUNC_EXTEND);
    SHOW_DEFINE(HGL_VECTOR_FUNC_EXTEND_ARRAY);
    SHOW_DEFINE(HGL_VECTOR_GROWTH_TYPE);
    SHOW_DEFINE(EXPONENTIAL);
    SHOW_DEFINE(HGL_VECTOR_LINEAR_GROWTH_RATE);
    SHOW_DEFINE(HGL_VECTOR_EXPONENTIAL_GROWTH_RATE);
#endif

    hgl_int_vector_t v;
    hgl_int_vector_t v2;
    hgl_float_vector_t vf;

#if 1
    hgl_float_vector_init(&vf);
    hgl_float_vector_push_value(&vf, 23.4f);
    hgl_float_vector_push_value(&vf, 20.4f);
    hgl_float_vector_push_value(&vf, 8.4f);
    hgl_float_vector_push_value(&vf, 27.4f);
    hgl_float_vector_pop(&vf);
    hgl_float_vector_free(&vf);
#endif

#if 1
    int arr[] = {4, 2, 0};
    hgl_int_vector_init_capacity(&v, 2);
    hgl_int_vector_init(&v2);
    hgl_int_vector_extend_array(&v, arr, 3);
    hgl_int_vector_shrink_to_fit(&v);
    hgl_int_vector_push_value(&v, 1);
    hgl_int_vector_push_value(&v, 3);
    hgl_int_vector_push_value(&v, 3);
    hgl_int_vector_push_value(&v, 7);
    hgl_int_vector_push_value(&v2, 4);
    hgl_int_vector_push_value(&v2, 2);
    hgl_int_vector_push_value(&v2, 0);
    hgl_int_vector_push_value(&v2, 6);
    hgl_int_vector_push_value(&v2, 9);
    
    hgl_int_vector_insert(&v, &v2, 2);

    hgl_int_vector_free(&v);
    hgl_int_vector_free(&v2);
#endif
#if 1
    hgl_int_vector_init(&v);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vector_push_value(&v, 1);
    int asd = 3;
    hgl_int_vector_push(&v, &asd);
    hgl_int_vector_push_value(&v, 3);
    hgl_int_vector_push_value(&v, 7);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);

    hgl_int_vector_init(&v2);
    hgl_int_vector_push_value(&v2, 4);
    hgl_int_vector_push_value(&v2, 2);
    hgl_int_vector_push_value(&v2, 0);

    hgl_int_vector_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vector_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vector_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);
    hgl_int_vector_extend(&v, &v2);
    printf("vector -- len: %lu capacity: %lu\n", v.len, v.capacity);

    hgl_int_vector_pop(&v);
    hgl_int_vector_pop(&v);
    int four = hgl_int_vector_pop(&v);
    printf("four: %d\n", four);

    for (size_t i = 0; i < v.len; i++) {
        printf("%d ", v.arr[i]);
    }
    printf("\n");
   
    hgl_int_vector_remove(&v, 0);
    hgl_int_vector_sort(&v, NULL);
    int seven = 7;
    int *resp = hgl_int_vector_bsearch(&v, &seven, NULL);

    printf("res: %p\n", (void*)resp);
    printf("arrbegin: %p\n", (void*)v.arr);

    for (size_t i = 0; i < v.len; i++) {
        printf("%d ", v.arr[i]);
    }
    printf("\n");

    printf("kalas\n");
    
    printf("v  arr: %p\n", (void *) v.arr);

    hgl_int_vector_free(&v);
    
    printf("v2 arr: %p\n", (void *) v2.arr);
    hgl_int_vector_free(&v2);
#endif

    //voidp_vector v;
    //printf("v size: %lu\n"v.size);
}
