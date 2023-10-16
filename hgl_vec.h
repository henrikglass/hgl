
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_vec.h implements a dynamic array similar to the vector class in C++.
 *
 *
 * USAGE:
 *
 * Include hgl_vec.h file like this:
 *
 *     #define HGL_VEC_TYPE char *
 *     #define HGL_VEC_TYPE_ID charp
 *     #include "hgl_vec.h"
 *
 * This will create an implementation of hgl_vec capable of holding elements of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_vec_init_capacity(hgl_charp_vec_t *vec, size_t initial_capacity);
 *     void hgl_charp_vec_init(hgl_charp_vec_t *vec);
 *     void hgl_charp_vec_free(hgl_charp_vec_t *vec);
 *     void hgl_charp_vec_reserve(hgl_charp_vec_t *vec, size_t new_capacity);
 *     void hgl_charp_vec_shrink_to_fit(hgl_charp_vec_t *vec);
 *     void hgl_charp_vec_push(hgl_charp_vec_t *vec, char **elem);
 *     void hgl_charp_vec_push_value(hgl_charp_vec_t *vec, char *elem);
 *     char *hgl_charp_vec_pop(hgl_charp_vec_t *vec);
 *     char *hgl_charp_vec_remove(hgl_charp_vec_t *vec, size_t index);
 *     char *hgl_charp_vec_insert(hgl_charp_vec_t *vec,
 *                                const hgl_charp_vec_t *other_vec,
 *                                size_t index);
 *     char *hgl_charp_vec_extend(hgl_charp_vec_t *vec, const hgl_charp_vec_t *other_vec);
 *     char *hgl_charp_vec_extend_array(hgl_charp_vec_t *vec, const *other_vec);
 *     void hgl_charp_vec_sort(hgl_charp_vec_t *vec,
 *                             int (cmpfnc*)(const void *, const void *));
 *     void hgl_charp_vec_bsearch(hgl_charp_vec_t *vec,
 *                                char **key,
 *                                int (cmpfnc*)(const void *, const void *));
 *
 * HGL_VEC_TYPE and HGL_VEC_TYPE_ID may be redefined and hgl_vec.h included multiple times
 * to create implementations of hgl_vec for different types:
 *
 *     #define HGL_VEC_TYPE char *
 *     #define HGL_VEC_TYPE_ID charp
 *     #include "hgl_vec.h"
 *
 *     #undef HGL_VEC_TYPE
 *     #undef HGL_VEC_TYPE_ID
 *     #define HGL_VEC_TYPE int
 *     #define HGL_VEC_TYPE_ID int
 *     #include "hgl_vec.h"
 *
 * If HGL_VEC_TYPE and HGL_VEC_TYPE_ID are left undefined, the default element type of
 * hgl_vec will be void *, and the default element type identifier will be "voidp".
 *
 * hgl_vec allows fine-grained control over the internals of the implementation
 * including configurable growth rate and growth type (linear or exponential), as well
 * as support for custom allocators. The following defines (shown with their default
 * values) may be changed before including hgl_vec.h:
 *
 *     #define HGL_VEC_GROWTH_TYPE              HGL_VEC_EXPONENTIAL
 *     #define HGL_VEC_EXPONENTIAL_GROWTH_RATE  1.5
 *     #define HGL_VEC_LINEAR_GROWTH_RATE       64
 *     #define HGL_VEC_ALLOCATOR                (malloc)
 *     #define HGL_VEC_REALLOCATOR              (realloc)
 *     #define HGL_VEC_FREE                     (free)
 *
 * HGL_VEC_GROWTH_TYPE can take on the values HGL_VEC_EXPONENTIAL and HGL_VEC_LINEAR.
 * HGL_VEC_EXPONENTIAL_GROWTH_RATE and HGL_VEC_LINEAR_GROWTH_RATE can take on any numeric
 * literal. HGL_VEC_ALLOCATOR, HGL_VEC_REALLOCATOR, and HGL_VEC_FREE can take on any
 * function identifiers whose function types match those of malloc, realloc, and free,
 * respectively.
 *
 * AUTHOR: Henrik A. Glass
 *
 */

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
#define HGL_VEC_EXPONENTIAL_GROWTH_RATE 1.5
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

#define HGL_VEC_FUNC_DEFAULT_CMPFNC_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_default_cmpfnc_)
int HGL_VEC_FUNC_DEFAULT_CMPFNC_NAME(const void * a, const void * b)
{
    /*
     * NOTE: this works as a default for built in arithmetic types. For more complex
     *       types, supply your own compare function to hgl_vec_*_sort(...) instead
     */
    HGL_VEC_TYPE a_ = *(const HGL_VEC_TYPE*) a;
    HGL_VEC_TYPE b_ = *(const HGL_VEC_TYPE*) b;
    return (a_ > b_) - (a_ < b_);
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
static inline void HGL_VEC_FUNC_PUSH_NAME(HGL_VEC_STRUCT_NAME *vec, HGL_VEC_TYPE *elem)
{
    if (vec->len == vec->capacity) {
        HGL_VEC_FUNC_GROW_VEC_NAME(vec);
    }

    vec->arr[vec->len++] = *elem;
}

#define HGL_VEC_FUNC_PUSH_VALUE_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_push_value)
static inline void HGL_VEC_FUNC_PUSH_VALUE_NAME(HGL_VEC_STRUCT_NAME *vec, HGL_VEC_TYPE elem)
{
    HGL_VEC_FUNC_PUSH_NAME(vec, &elem);
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

#define HGL_VEC_FUNC_SORT_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_sort)
static inline void HGL_VEC_FUNC_SORT_NAME(HGL_VEC_STRUCT_NAME *vec,
                                          int (*cmpfnc)(const void *, const void *))
{
    /*
     * No compare function supplied, attempt to use the default one... This will only work
     * for simple built-in arithmetic types (char, int, float, ...)
     */
    if (cmpfnc == NULL) {
        cmpfnc = &HGL_VEC_FUNC_DEFAULT_CMPFNC_NAME;
    }

    qsort(vec->arr, vec->len, sizeof(HGL_VEC_TYPE), cmpfnc);

}

#define HGL_VEC_FUNC_BSEARCH_NAME _CONCAT3(hgl_, HGL_VEC_TYPE_ID, _vec_bsearch)
static inline HGL_VEC_TYPE *HGL_VEC_FUNC_BSEARCH_NAME(HGL_VEC_STRUCT_NAME *vec,
                                                      HGL_VEC_TYPE *key,
                                                      int (*cmpfnc)(const void *, const void *))
{
    /*
     * No compare function supplied, attempt to use the default one... This will only work
     * for simple built-in arithmetic types (char, int, float, ...)
     */
    if (cmpfnc == NULL) {
        cmpfnc = &HGL_VEC_FUNC_DEFAULT_CMPFNC_NAME;
    }

    return bsearch(key, vec->arr, vec->len, sizeof(HGL_VEC_TYPE), cmpfnc);

}

