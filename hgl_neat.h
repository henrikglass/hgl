/**
 * Just a collection of neat stuff.
 */

#ifndef HGL_NEAT_H
#define HGL_NEAT_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

/*--- Public macros ---------------------------------------------------------------------*/

/* zig-style try macro */
#define TRY(expr)                   \
    do {                            \
        int err_ = (int) (expr);    \
        if (err_ != 0) {            \
            return err_;            \
        }                           \
    } while (0)

/* technically UB, but works in a pinch */
#define TRANSMUTE(T, var) (*(T *) &var)

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
static inline void show_build_info(void)
{
    extern const char *__progname;
    printf("-------------------------------------------------- \n");
    printf("Executable: %s\nBuilt on: %s %s\n", __progname, __DATE__, __TIME__);
    printf("-------------------------------------------------- \n");
}

#endif /* HGL_NEAT_H */

