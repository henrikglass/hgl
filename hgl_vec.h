

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

/*--- vector-specific macros ------------------------------------------------------------*/

/* CONFIGURABLE: HGL_VEC_TYPE & HGL_VEC_TYPE_ID */
#ifndef HGL_VEC_TYPE
#define HGL_VEC_TYPE void *
#define HGL_VEC_TYPE_ID voidp
#endif /* HGL_VEC_TYPE */

/* CONFIGURABLE: HGL_VEC_GROWTH_TYPE */
#ifndef HGL_VEC_GROWTH_TYPE
#define HGL_VEC_GROWTH_TYPE HGL_VEC_EXPONENTIAL
//#define HGL_VEC_GROWTH_TYPE HGL_VEC_LINEAR
#endif

/* CONFIGURABLE: HGL_VEC_EXPONENTIAL_GROWTH_RATE */
#ifndef HGL_VEC_EXPONENTIAL_GROWTH_RATE
#define HGL_VEC_EXPONENTIAL_GROWTH_RATE 2
#endif

/* CONFIGURABLE: HGL_VEC_LINEAR_GROWTH_RATE */
#ifndef HGL_VEC_LINEAR_GROWTH_RATE
#define HGL_VEC_LINEAR_GROWTH_RATE 64
#endif

/* CONFIGURABLE: HGL_VEC_ALLOCATOR, HGL_VEC_REALLOCATOR, HGL_VEC_FREE */
#if !defined(HGL_VEC_ALLOCATOR) && !defined(HGL_VEC_REALLOCATOR) && !defined(HGL_VEC_FREE)
#define HGL_VEC_ALLOCATOR (malloc)
#define HGL_VEC_REALLOCATOR (realloc)
#define HGL_VEC_FREE (free)
#endif

/* Should be left untouched */
#define _DEFAULT_INITIAL_CAPACITY 10

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_VEC_STRUCT_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_t)
typedef struct
{
    HGL_VEC_TYPE *arr;
    size_t len;
    size_t capacity;
} HGL_VEC_STRUCT_NAME;

/*--- Public variables ------------------------------------------------------------------*/

/*--- helper functions ------------------------------------------------------------------*/

#define HGL_VEC_FUNC_GROW_VEC_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_grow_vec_)
static inline void HGL_VEC_FUNC_GROW_VEC_NAME(HGL_VEC_STRUCT_NAME *vec)
{
#if HGL_VEC_GROWTH_TYPE == HGL_VEC_EXPONENTIAL
        vec->capacity *= HGL_VEC_EXPONENTIAL_GROWTH_RATE;
#elif HGL_VEC_GROWTH_TYPE == HGL_VEC_LINEAR
        vec->capacity += HGL_VEC_LINEAR_GROWTH_RATE;
#endif
        vec->arr = HGL_VEC_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VEC_TYPE));
}

/*--- vector functions ------------------------------------------------------------------*/

#define HGL_VEC_FUNC_INIT_CAPACITY_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_init_capacity)
static inline void HGL_VEC_FUNC_INIT_CAPACITY_NAME(HGL_VEC_STRUCT_NAME *vec, size_t initial_capacity)
{
    vec->len         = 0;
    vec->capacity    = initial_capacity;
    vec->arr         = HGL_VEC_ALLOCATOR(vec->capacity * sizeof(HGL_VEC_TYPE));
}

#define HGL_VEC_FUNC_INIT_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_init)
static inline void HGL_VEC_FUNC_INIT_NAME(HGL_VEC_STRUCT_NAME *vec)
{
    HGL_VEC_FUNC_INIT_CAPACITY_NAME(vec, _DEFAULT_INITIAL_CAPACITY);
}

#define HGL_VEC_FUNC_FREE_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_free)
static inline void HGL_VEC_FUNC_FREE_NAME(HGL_VEC_STRUCT_NAME *vec)
{
    HGL_VEC_FREE(vec->arr);
}

#define HGL_VEC_FUNC_RESERVE_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_reserve)
static inline void HGL_VEC_FUNC_RESERVE_NAME(HGL_VEC_STRUCT_NAME *vec, size_t new_capacity)
{
    if (new_capacity < vec->capacity) {
        return;
    }
    vec->capacity = new_capacity;
    vec->arr = HGL_VEC_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VEC_TYPE));
}

#define HGL_VEC_FUNC_SHRINK_TO_FIT_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_shrink_to_fit)
static inline void HGL_VEC_FUNC_SHRINK_TO_FIT_NAME(HGL_VEC_STRUCT_NAME *vec)
{
    vec->capacity = vec->len;
    vec->arr = HGL_VEC_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VEC_TYPE));
}

#define HGL_VEC_FUNC_PUSH_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_push)
static inline void HGL_VEC_FUNC_PUSH_NAME(HGL_VEC_STRUCT_NAME *vec, HGL_VEC_TYPE elem)
{
    if (vec->len == vec->capacity) {
        HGL_VEC_FUNC_GROW_VEC_NAME(vec);
    }
    
    vec->arr[vec->len++] = elem; 
}

#define HGL_VEC_FUNC_POP_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_pop)
static inline HGL_VEC_TYPE HGL_VEC_FUNC_POP_NAME(HGL_VEC_STRUCT_NAME *vec)
{
    assert(vec->len > 0 && "tried to pop from a vector of size 0");
    return vec->arr[--vec->len]; 
}

#define HGL_VEC_FUNC_REMOVE_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_remove)
static inline HGL_VEC_TYPE HGL_VEC_FUNC_REMOVE_NAME(HGL_VEC_STRUCT_NAME *vec, size_t index)
{
    assert(index < vec->len && "tried to remove element at out-of-bounds index");
    HGL_VEC_TYPE retval = vec->arr[index];
    memmove(&vec->arr[index], &vec->arr[index+1], sizeof(HGL_VEC_TYPE)*(vec->len - index));
    vec->len--;
    return retval; 
}

#define HGL_VEC_FUNC_INSERT_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_insert)
static inline void HGL_VEC_FUNC_INSERT_NAME(HGL_VEC_STRUCT_NAME *vec, 
                                            const HGL_VEC_STRUCT_NAME *other_vec,
                                            size_t index)
{
    while (vec->len + other_vec->len > vec->capacity) {
        HGL_VEC_FUNC_GROW_VEC_NAME(vec);
    }

    /* 
     * Do the simpler (but in some cases probably a lot slower) version. 
     *
     * TODO: We could get away with a smaller temporary buffer with some clever tricks.
     */

    /* allocate temporary buffer */
    size_t temp_buf_size = sizeof(HGL_VEC_TYPE) * (vec->len - index);
    HGL_VEC_TYPE *temp_buf = HGL_VEC_ALLOCATOR(temp_buf_size);
    
    /* copy displaced elements (+ end) into temporary buffer */
    memcpy(temp_buf, &vec->arr[index], temp_buf_size);

    /* write new elements into vec->arr at specified index */    
    memcpy(&vec->arr[index], other_vec->arr, other_vec->len * sizeof(HGL_VEC_TYPE));

    /* write temporary buffer to end of newly inserted elements */
    memcpy(&vec->arr[index + other_vec->len], temp_buf, temp_buf_size);

    /* update vec length */
    vec->len += other_vec->len;
    
    /* free temporary buffer */
    HGL_VEC_FREE(temp_buf);
}

#define HGL_VEC_FUNC_EXTEND_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_extend)
static inline void HGL_VEC_FUNC_EXTEND_NAME(HGL_VEC_STRUCT_NAME *vec, 
                                            const HGL_VEC_STRUCT_NAME *other_vec)
{
    while (vec->len + other_vec->len > vec->capacity) {
        HGL_VEC_FUNC_GROW_VEC_NAME(vec);
    }
    memcpy(&vec->arr[vec->len], other_vec->arr, sizeof(HGL_VEC_TYPE)*other_vec->len);
    vec->len += other_vec->len;
}

#define HGL_VEC_FUNC_EXTEND_ARRAY_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_extend_array)
static inline void HGL_VEC_FUNC_EXTEND_ARRAY_NAME(HGL_VEC_STRUCT_NAME *vec,
                                                  const HGL_VEC_TYPE other_arr[],
                                                  size_t other_arr_len) 
{
    while (vec->len + other_arr_len > vec->capacity) {
        HGL_VEC_FUNC_GROW_VEC_NAME(vec);
    }
    memcpy(&vec->arr[vec->len], other_arr, sizeof(HGL_VEC_TYPE)*other_arr_len);
    vec->len += other_arr_len;
}

