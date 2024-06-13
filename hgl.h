
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
 * hgl.h contains a bunch of misc. typedefs, macros, and useful functions that I 
 * use from time to time.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */
#ifndef HGL_H
#define HGL_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

/*--- Public macros ---------------------------------------------------------------------*/

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

__attribute__((always_inline))
static inline void hgl_stack_trace()
{
    void *array[32];
    char **strings;

    int size = backtrace(array, 32);
    strings = backtrace_symbols(array, size);
    //backtrace_symbols_fd(array, size, STDERR_FILENO);
    if (strings != NULL) {
        printf ("Stack trace (compile with -rdynamic to enable symbols): \n");
        for (int i = 0; i < size; i++) {
            printf ("  [%d] %s\n", i, strings[i]);
        }
    }

    free(strings);
}

#endif /* HGL_H */

