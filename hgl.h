
#ifndef HGL_H
#define HGL_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*--- Public macros ---------------------------------------------------------------------*/

#if !defined(HGL_ALLOC) && \
    !defined(HGL_REALLOC) && \
    !defined(HGL_FREE)
#define HGL_ALLOC malloc
#define HGL_REALLOC realloc
#define HGL_FREE free
#endif

/* zig-style try macro */
#define try(expr)                   \
    do {                            \
        int err_ = (int) (expr);    \
        if (err_ != 0) {            \
            return err_;            \
        }                           \
    } while (0)

/* technically UB, but works in a pinch */
#define transmute(T, var) (*(T *) &(var))

#ifndef HGL_ISO_C
#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})
#else
#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)
#endif

/*
 * General purpose minimal dynamic array. Just create a struct of the correct form
 * by using the HglDynamicArray(T) macro:
 *
 * typedef HglDynamicArray(float) Floats;
 * Floats fs = {0};
 * hgl_da_append(&fs, 123.456f);
 *
 * or:
 *
 * HglDynamicArray(float) fs = {0};
 * hgl_da_append(&fs, 123.456f);
 *
 */

#define HglDynamicArray(T) \
    struct {               \
        T *arr;            \
        size_t length;     \
        size_t capacity;   \
    }

#ifndef HGL_DA_INITIAL_CAPACITY
#define HGL_DA_INITIAL_CAPACITY 64
#endif

#ifndef HGL_DA_GROWTH_RATE
#define HGL_DA_GROWTH_RATE 1.5
#endif

#define hgl_da_push(da, item)                                                        \
    do {                                                                             \
        if ((da)->arr == NULL) {                                                     \
            (da)->length = 0;                                                        \
            (da)->capacity = HGL_DA_INITIAL_CAPACITY;                                \
            (da)->arr = HGL_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                            \
        if ((da)->capacity < ((da)->length + 1)) {                                   \
            (da)->capacity *= HGL_DA_GROWTH_RATE;                                    \
            (da)->arr = HGL_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                            \
        assert(((da)->arr != NULL) && "[hgl] Error: (re)alloc failed");              \
        (da)->arr[(da)->length++] = (item);                                          \
    } while (0)

#define hgl_da_pop(da) ((da)->arr[--(da)->length])

#define hgl_da_extend(da, items, n)                                                  \
    do {                                                                             \
        if ((da)->arr == NULL) {                                                     \
            (da)->length = 0;                                                        \
            (da)->capacity = max(HGL_DA_INITIAL_CAPACITY, (int)next_pow2_((n)));     \
            (da)->arr = HGL_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                            \
        if ((da)->capacity < ((da)->length + (n))) {                                 \
            while ((da)->capacity < ((da)->length + (n))) {                          \
                (da)->capacity *= HGL_DA_GROWTH_RATE;                                \
            }                                                                        \
            (da)->arr = HGL_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                            \
        assert(((da)->arr != NULL) && "[hgl] Error: (re)alloc failed");              \
        memcpy(&(da)->arr[(da)->length], (items), (n)*sizeof(*(da)->arr));           \
        (da)->length += (n);                                                         \
    } while (0)

#define hgl_da_reserve(da, new_capacity)                                             \
    do {                                                                             \
        if ((da)->arr == NULL) {                                                     \
            (da)->length = 0;                                                        \
            (da)->capacity = max(HGL_DA_INITIAL_CAPACITY, (int)new_capacity);        \
            (da)->arr = HGL_ALLOC((da)->capacity * sizeof(*(da)->arr));              \
        }                                                                            \
        if ((da)->capacity >= (new_capacity)) break;                                 \
        (da)->capacity = (new_capacity);                                             \
        (da)->arr = HGL_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr));     \
    } while (0)

#define hgl_da_free(da)                                                              \
    do {                                                                             \
        HGL_FREE((da)->arr);                                                         \
    } while (0)

#define hgl_da_insert(da, index, item)                                               \
    do {                                                                             \
        assert((ssize_t)(index) >= 0 && (index) < (da)->length);                     \
        if ((da)->capacity < ((da)->length + 1)) {                                   \
            (da)->capacity *= HGL_DA_GROWTH_RATE;                                    \
            (da)->arr = HGL_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                            \
        memmove(&((da)->arr[(index) + 1]),                                           \
                &((da)->arr[(index)]),                                               \
                ((da)->length - (index))* sizeof(*(da)->arr));                       \
        (da)->length++;                                                              \
        (da)->arr[(index)] = item;                                                   \
    } while (0)

#ifndef HGL_ISO_C

#define hgl_da_remove(da, index)                                                     \
    ({                                                                               \
        assert((ssize_t)(index) >= 0 && (index) < (da)->length);                     \
        __typeof__(*(da)->arr) item = (da)->arr[(index)];                            \
        memmove(&((da)->arr[(index)]),                                               \
                &((da)->arr[(index) + 1]),                                           \
                ((da)->length - (index))* sizeof(*(da)->arr));                       \
        (da)->length--;                                                              \
        item;                                                                        \
    })

#endif

/*--- Public type definitions -----------------------------------------------------------*/

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

/* just in case... */
static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

/*--- Public functions ------------------------------------------------------------------*/

/* Show executable name and build timestamp */
static inline void hgl_show_build_info(void)
{
    extern const char *__progname;
    printf("-------------------------------------------------- \n");
    printf("Executable: %s\nBuilt on: %s %s\n", __progname, __DATE__, __TIME__);
    printf("-------------------------------------------------- \n");
}

/* helper for hgl_da_extend */
static inline uint32_t next_pow2_(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#endif /* HGL_H */

