
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
 * hgl_vector.h implements a dynamic array similar to the vector class in C++.
 *
 *
 * USAGE:
 *
 * Include hgl_vector.h file like this:
 *
 *     #define HGL_VECTOR_TYPE char *
 *     #define HGL_VECTOR_TYPE_ID charp
 *     #include "hgl_vector.h"
 *
 * This will create an implementation of hgl_vector capable of holding elements of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_vector_init_capacity(hgl_charp_vector_t *vec, size_t initial_capacity);
 *     void hgl_charp_vector_init(hgl_charp_vector_t *vec);
 *     void hgl_charp_vector_free(hgl_charp_vector_t *vec);
 *     void hgl_charp_vector_reserve(hgl_charp_vector_t *vec, size_t new_capacity);
 *     void hgl_charp_vector_shrink_to_fit(hgl_charp_vector_t *vec);
 *     void hgl_charp_vector_push(hgl_charp_vector_t *vec, char **elem);
 *     void hgl_charp_vector_push_value(hgl_charp_vector_t *vec, char *elem);
 *     char *hgl_charp_vector_pop(hgl_charp_vector_t *vec);
 *     char *hgl_charp_vector_remove(hgl_charp_vector_t *vec, size_t index);
 *     char *hgl_charp_vector_insert(hgl_charp_vector_t *vec,
 *                                   const hgl_charp_vector_t *other_vec,
 *                                   size_t index);
 *     char *hgl_charp_vector_extend(hgl_charp_vector_t *vec, const hgl_charp_vector_t *other_vec);
 *     char *hgl_charp_vector_extend_array(hgl_charp_vector_t *vec, const *other_arr);
 *     void hgl_charp_vector_sort(hgl_charp_vector_t *vec,
 *                                int (cmpfnc*)(const void *, const void *));
 *     void hgl_charp_vector_bsearch(hgl_charp_vector_t *vec,
 *                                   char **key,
 *                                   int (cmpfnc*)(const void *, const void *));
 *
 * HGL_VECTOR_TYPE and HGL_VECTOR_TYPE_ID may be redefined and hgl_vector.h included multiple times
 * to create implementations of hgl_vector for different types:
 *
 *     #define HGL_VECTOR_TYPE char *
 *     #define HGL_VECTOR_TYPE_ID charp
 *     #include "hgl_vector.h"
 *
 *     #undef HGL_VECTOR_TYPE
 *     #undef HGL_VECTOR_TYPE_ID
 *     #define HGL_VECTOR_TYPE int
 *     #define HGL_VECTOR_TYPE_ID int
 *     #include "hgl_vector.h"
 *
 * If HGL_VECTOR_TYPE and HGL_VECTOR_TYPE_ID are left undefined, the default element type of
 * hgl_vector will be void *, and the default element type identifier will be "voidp".
 *
 * hgl_vector allows fine-grained control over the internals of the implementation
 * including configurable growth rate and growth type (linear or exponential), as well
 * as support for custom allocators. The following defines (shown with their default
 * values) may be changed before including hgl_vector.h:
 *
 *     #define HGL_VECTOR_GROWTH_TYPE              HGL_VECTOR_EXPONENTIAL
 *     #define HGL_VECTOR_EXPONENTIAL_GROWTH_RATE  1.5
 *     #define HGL_VECTOR_LINEAR_GROWTH_RATE       64
 *     #define HGL_VECTOR_ALLOCATOR                (malloc)
 *     #define HGL_VECTOR_REALLOCATOR              (realloc)
 *     #define HGL_VECTOR_FREE                     (free)
 *
 * HGL_VECTOR_GROWTH_TYPE can take on the values HGL_VECTOR_EXPONENTIAL and HGL_VECTOR_LINEAR.
 * HGL_VECTOR_EXPONENTIAL_GROWTH_RATE and HGL_VECTOR_LINEAR_GROWTH_RATE can take on any numeric
 * literal. HGL_VECTOR_ALLOCATOR, HGL_VECTOR_REALLOCATOR, and HGL_VECTOR_FREE can take on any
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

/* CONFIGURABLE: HGL_VECTOR_TYPE & HGL_VECTOR_TYPE_ID */
#ifndef HGL_VECTOR_TYPE
#define HGL_VECTOR_TYPE void *
#define HGL_VECTOR_TYPE_ID voidp
#endif /* HGL_VECTOR_TYPE */

/* CONFIGURABLE: HGL_VECTOR_GROWTH_TYPE */
#ifndef HGL_VECTOR_GROWTH_TYPE
#define HGL_VECTOR_GROWTH_TYPE HGL_VECTOR_EXPONENTIAL
//#define HGL_VECTOR_GROWTH_TYPE HGL_VECTOR_LINEAR
#endif

/* CONFIGURABLE: HGL_VECTOR_EXPONENTIAL_GROWTH_RATE */
#ifndef HGL_VECTOR_EXPONENTIAL_GROWTH_RATE
#define HGL_VECTOR_EXPONENTIAL_GROWTH_RATE 1.5
#endif

/* CONFIGURABLE: HGL_VECTOR_LINEAR_GROWTH_RATE */
#ifndef HGL_VECTOR_LINEAR_GROWTH_RATE
#define HGL_VECTOR_LINEAR_GROWTH_RATE 64
#endif

/* CONFIGURABLE: HGL_VECTOR_ALLOCATOR, HGL_VECTOR_REALLOCATOR, HGL_VECTOR_FREE */
#if !defined(HGL_VECTOR_ALLOCATOR) && \
    !defined(HGL_VECTOR_REALLOCATOR) && \
    !defined(HGL_VECTOR_FREE)
#define HGL_VECTOR_ALLOCATOR (malloc)
#define HGL_VECTOR_REALLOCATOR (realloc)
#define HGL_VECTOR_FREE (free)
#endif

/* Should be left untouched */
#define _DEFAULT_INITIAL_CAPACITY 10

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_VECTOR_STRUCT _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_t)
typedef struct
{
    HGL_VECTOR_TYPE *arr;
    size_t len;
    size_t capacity;
} HGL_VECTOR_STRUCT;

/*--- Public variables ------------------------------------------------------------------*/

/*--- helper functions ------------------------------------------------------------------*/

#define HGL_VECTOR_FUNC_GROW _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_grow_)
static inline void HGL_VECTOR_FUNC_GROW(HGL_VECTOR_STRUCT *vec)
{
#if HGL_VECTOR_GROWTH_TYPE == HGL_VECTOR_EXPONENTIAL
        vec->capacity *= HGL_VECTOR_EXPONENTIAL_GROWTH_RATE;
#elif HGL_VECTOR_GROWTH_TYPE == HGL_VECTOR_LINEAR
        vec->capacity += HGL_VECTOR_LINEAR_GROWTH_RATE;
#endif
        vec->arr = HGL_VECTOR_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VECTOR_TYPE));
}

#define HGL_VECTOR_FUNC_DEFAULT_CMPFNC _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_default_cmpfnc_)
int HGL_VECTOR_FUNC_DEFAULT_CMPFNC(const void * a, const void * b)
{
    /*
     * NOTE: this works as a default for built in arithmetic types. For more complex
     *       types, supply your own compare function to hgl_vector_*_sort(...) instead
     */
    HGL_VECTOR_TYPE a_ = *(const HGL_VECTOR_TYPE*) a;
    HGL_VECTOR_TYPE b_ = *(const HGL_VECTOR_TYPE*) b;
    return (a_ > b_) - (a_ < b_);
}

/*--- vector functions ------------------------------------------------------------------*/

#define HGL_VECTOR_FUNC_INIT_CAPACITY _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_init_capacity)
static inline void HGL_VECTOR_FUNC_INIT_CAPACITY(HGL_VECTOR_STRUCT *vec, size_t initial_capacity)
{
    vec->len         = 0;
    vec->capacity    = initial_capacity;
    vec->arr         = HGL_VECTOR_ALLOCATOR(vec->capacity * sizeof(HGL_VECTOR_TYPE));
}

#define HGL_VECTOR_FUNC_INIT _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_init)
static inline void HGL_VECTOR_FUNC_INIT(HGL_VECTOR_STRUCT *vec)
{
    HGL_VECTOR_FUNC_INIT_CAPACITY(vec, _DEFAULT_INITIAL_CAPACITY);
}

#define HGL_VECTOR_FUNC_FREE _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_free)
static inline void HGL_VECTOR_FUNC_FREE(HGL_VECTOR_STRUCT *vec)
{
    HGL_VECTOR_FREE(vec->arr);
}

#define HGL_VECTOR_FUNC_RESERVE _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_reserve)
static inline void HGL_VECTOR_FUNC_RESERVE(HGL_VECTOR_STRUCT *vec, size_t new_capacity)
{
    if (new_capacity < vec->capacity) {
        return;
    }
    vec->capacity = new_capacity;
    vec->arr = HGL_VECTOR_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VECTOR_TYPE));
}

#define HGL_VECTOR_FUNC_SHRINK_TO_FIT _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_shrink_to_fit)
static inline void HGL_VECTOR_FUNC_SHRINK_TO_FIT(HGL_VECTOR_STRUCT *vec)
{
    vec->capacity = vec->len;
    vec->arr = HGL_VECTOR_REALLOCATOR(vec->arr, vec->capacity * sizeof(HGL_VECTOR_TYPE));
}

#define HGL_VECTOR_FUNC_PUSH _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_push)
static inline void HGL_VECTOR_FUNC_PUSH(HGL_VECTOR_STRUCT *vec, HGL_VECTOR_TYPE *elem)
{
    if (vec->len == vec->capacity) {
        HGL_VECTOR_FUNC_GROW(vec);
    }

    vec->arr[vec->len++] = *elem;
}

#define HGL_VECTOR_FUNC_PUSH_VALUE _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_push_value)
static inline void HGL_VECTOR_FUNC_PUSH_VALUE(HGL_VECTOR_STRUCT *vec, HGL_VECTOR_TYPE elem)
{
    HGL_VECTOR_FUNC_PUSH(vec, &elem);
}

#define HGL_VECTOR_FUNC_POP _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_pop)
static inline HGL_VECTOR_TYPE HGL_VECTOR_FUNC_POP(HGL_VECTOR_STRUCT *vec)
{
    assert(vec->len > 0 && "tried to pop from a vector of size 0");
    return vec->arr[--vec->len];
}

#define HGL_VECTOR_FUNC_REMOVE _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_remove)
static inline HGL_VECTOR_TYPE HGL_VECTOR_FUNC_REMOVE(HGL_VECTOR_STRUCT *vec, size_t index)
{
    assert(index < vec->len && "tried to remove element at out-of-bounds index");
    HGL_VECTOR_TYPE retval = vec->arr[index];
    memmove(&vec->arr[index], &vec->arr[index+1], sizeof(HGL_VECTOR_TYPE)*(vec->len - index));
    vec->len--;
    return retval;
}

#define HGL_VECTOR_FUNC_INSERT _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_insert)
static inline void HGL_VECTOR_FUNC_INSERT(HGL_VECTOR_STRUCT *vec,
                                          const HGL_VECTOR_STRUCT *other_vec,
                                          size_t index)
{
    while (vec->len + other_vec->len > vec->capacity) {
        HGL_VECTOR_FUNC_GROW(vec);
    }

    /*
     * Do the simpler (but in some cases probably a lot slower) version.
     *
     * TODO: We could get away with a smaller temporary buffer with some clever tricks.
     */

    /* allocate temporary buffer */
    size_t temp_buf_size = sizeof(HGL_VECTOR_TYPE) * (vec->len - index);
    HGL_VECTOR_TYPE *temp_buf = HGL_VECTOR_ALLOCATOR(temp_buf_size);

    /* copy displaced elements (+ end) into temporary buffer */
    memcpy(temp_buf, &vec->arr[index], temp_buf_size);

    /* write new elements into vec->arr at specified index */
    memcpy(&vec->arr[index], other_vec->arr, other_vec->len * sizeof(HGL_VECTOR_TYPE));

    /* write temporary buffer to end of newly inserted elements */
    memcpy(&vec->arr[index + other_vec->len], temp_buf, temp_buf_size);

    /* update vec length */
    vec->len += other_vec->len;

    /* free temporary buffer */
    HGL_VECTOR_FREE(temp_buf);
}

#define HGL_VECTOR_FUNC_EXTEND _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_extend)
static inline void HGL_VECTOR_FUNC_EXTEND(HGL_VECTOR_STRUCT *vec,
                                          const HGL_VECTOR_STRUCT *other_vec)
{
    while (vec->len + other_vec->len > vec->capacity) {
        HGL_VECTOR_FUNC_GROW(vec);
    }
    memcpy(&vec->arr[vec->len], other_vec->arr, sizeof(HGL_VECTOR_TYPE)*other_vec->len);
    vec->len += other_vec->len;
}

#define HGL_VECTOR_FUNC_EXTEND_ARRAY _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_extend_array)
static inline void HGL_VECTOR_FUNC_EXTEND_ARRAY(HGL_VECTOR_STRUCT *vec,
                                                const HGL_VECTOR_TYPE other_arr[],
                                                size_t other_arr_len)
{
    while (vec->len + other_arr_len > vec->capacity) {
        HGL_VECTOR_FUNC_GROW(vec);
    }
    memcpy(&vec->arr[vec->len], other_arr, sizeof(HGL_VECTOR_TYPE)*other_arr_len);
    vec->len += other_arr_len;
}

#define HGL_VECTOR_FUNC_SORT _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_sort)
static inline void HGL_VECTOR_FUNC_SORT(HGL_VECTOR_STRUCT *vec,
                                        int (*cmpfnc)(const void *, const void *))
{
    /*
     * No compare function supplied, attempt to use the default one... This will only work
     * for simple built-in arithmetic types (char, int, float, ...)
     */
    if (cmpfnc == NULL) {
        cmpfnc = &HGL_VECTOR_FUNC_DEFAULT_CMPFNC;
    }

    qsort(vec->arr, vec->len, sizeof(HGL_VECTOR_TYPE), cmpfnc);

}

#define HGL_VECTOR_FUNC_BSEARCH _CONCAT3(hgl_, HGL_VECTOR_TYPE_ID, _vector_bsearch)
static inline HGL_VECTOR_TYPE *HGL_VECTOR_FUNC_BSEARCH(HGL_VECTOR_STRUCT *vec,
                                                       HGL_VECTOR_TYPE *key,
                                                       int (*cmpfnc)(const void *, const void *))
{
    /*
     * No compare function supplied, attempt to use the default one... This will only work
     * for simple built-in arithmetic types (char, int, float, ...)
     */
    if (cmpfnc == NULL) {
        cmpfnc = &HGL_VECTOR_FUNC_DEFAULT_CMPFNC;
    }

    return bsearch(key, vec->arr, vec->len, sizeof(HGL_VECTOR_TYPE), cmpfnc);

}

