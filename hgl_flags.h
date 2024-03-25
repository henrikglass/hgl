
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
 * hgl_flags.h implements a simple command-line flag/option parsing utility.
 *
 *
 * USAGE:
 * 
 * Include hgl_flags.h file like this:
 *
 *     #define HGL_FLAGS_IMPLEMENTATION
 *     #include "hgl_flags.h"
 *
 * The max number of allowed flags is 32 by default. To increase this, simply 
 * redefine HGL_FLAG_MAX_N_FLAGS before including hgl_flags.h.
 *
 * Code example:
 *
 *     bool *a = hgl_flags_add_bool("-a,--alternative-name", "Simple option for turning something on or off", false, 0);
 *     int *i = hgl_flags_add_int("-i", "Simple mandatory integer option", 123456, HGL_FLAG_OPT_MANDATORY);
 *     const char **outfile = hgl_flags_add_str("-o,--output", "Output file path", "a.out", 0);
 *
 *     int err = hgl_flags_parse(argc, argv);
 *     if (err != 0) {
 *         printf("Usage: %s [Options]\n", argv[0]);
 *         hgl_flags_print();
 *         return 1;
 *     }
 *
 *     printf("User provided i = %d\n");
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_FLAGS_H
#define HGL_FLAGS_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef HGL_FLAG_MAX_N_FLAGS
#define HGL_FLAG_MAX_N_FLAGS 32
#endif
#define HGL_FLAG_OPT_MANDATORY          0x00000001u

#define HGL_FLAG_STATUS_PARSED          0x00000001u
#define HGL_FLAG_STATUS_RANGE_OVERFLOW  0x00000002u
#define HGL_FLAG_STATUS_RANGE_UNDERFLOW 0x00000004u

static_assert(sizeof(long) == 8);
static_assert(sizeof(unsigned long) == 8);
static_assert(sizeof(double) == 8);

typedef enum
{
    HGL_FLAG_KIND_BOOL,
    HGL_FLAG_KIND_I64,
    HGL_FLAG_KIND_U64,
    HGL_FLAG_KIND_F64,
    HGL_FLAG_KIND_STR
} HglFlagKind;

typedef struct
{
    HglFlagKind kind;
    const char *names;
    uintptr_t default_value; // can fit any flag value type
    uintptr_t value;         // -- || --
    const char *desc;
    uint32_t opts;
    uint32_t status;
    uintptr_t range_min;
    uintptr_t range_max;
} HglFlag;

/**
 * Add a flag of type `bool`.
 */
bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts);

/**
 * Add a flag of type `int`, i.e. and `int64_t` with a valid value range of = [INT_MIN, INT_MAX].
 */
int *hgl_flags_add_int(const char *names, const char *desc, int default_value, uint32_t opts);

/**
 * Add a flag of type `int64_t`.
 */
int64_t *hgl_flags_add_i64(const char *names, const char *desc, int64_t default_value, uint32_t opts);

/**
 * Add a flag of type `int64_t` with specified range of valid values.
 */
int64_t *hgl_flags_add_i64_range(const char *names, const char *desc, int64_t default_value, 
                                 uint32_t opts, int64_t range_min, int64_t range_max);

/**
 * Add a flag of type `uint64_t`.
 */
uint64_t *hgl_flags_add_u64(const char *names, const char *desc, uint64_t default_value, uint32_t opts);

/**
 * Add a flag of type `uint64_t` with specified range of valid values.
 */
uint64_t *hgl_flags_add_u64_range(const char *names, const char *desc, uint64_t default_value, 
                                  uint32_t opts, uint64_t range_min, uint64_t range_max);

/**
 * Add a flag of type `double`.
 */
double *hgl_flags_add_f64(const char *names, const char *desc, double default_value, uint32_t opts);

/**
 * Add a flag of type `double` with specified range of valid values.
 */
double *hgl_flags_add_f64_range(const char *names, const char *desc, double default_value, 
                                uint32_t opts, double range_min, double range_max);

/**
 * Add a flag of type `const char *`.
 */
const char **hgl_flags_add_string(const char *names, const char *desc, const char *default_value, uint32_t opts);

/**
 * Parses all command line arguments.
 */
int hgl_flags_parse(int argc, char *argv[]);

/**
 * Prints the descriptions for all flags defined through calls to hgl_flags_add_*.
 */
void hgl_flags_print();

#endif

#ifdef HGL_FLAGS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)
#define transmute(T, var) (*(T *) &(var))

static HglFlag hgl_flags_[HGL_FLAG_MAX_N_FLAGS] = {0};
static size_t hgl_n_flags_ = 0;

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc, 
                         uintptr_t default_value, uint32_t opts, 
                         uintptr_t range_min, uintptr_t range_max);

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc, 
                         uintptr_t default_value, uint32_t opts, 
                         uintptr_t range_min, uintptr_t range_max)
{
    hgl_flags_[hgl_n_flags_++] = (HglFlag) {
        .kind          = kind,
        .names         = names,
        .default_value = default_value,
        .value         = default_value,
        .opts          = opts,
        .desc          = desc,
        .status        = 0,
        .range_min     = range_min,
        .range_max     = range_max,
    }; 
    return &hgl_flags_[hgl_n_flags_ - 1];
}

bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts)
{
    return (bool *) &hgl_flag_create_(HGL_FLAG_KIND_BOOL, names, desc, transmute(uintptr_t, default_value), 
                                      opts, 0, 0)->value;
} 

int *hgl_flags_add_int(const char *names, const char *desc, int default_value, uint32_t opts)
{
    int64_t default_value64 = (int64_t) default_value;
    return (int *) &hgl_flag_create_(HGL_FLAG_KIND_I64, names, desc, transmute(uintptr_t, default_value64), 
                                     opts, INT_MIN, INT_MAX)->value;
}

int64_t *hgl_flags_add_i64(const char *names, const char *desc, int64_t default_value, uint32_t opts)
{
    return (int64_t *) &hgl_flag_create_(HGL_FLAG_KIND_I64, names, desc, transmute(uintptr_t, default_value), 
                                         opts, LONG_MIN, LONG_MAX)->value;
}

int64_t *hgl_flags_add_i64_range(const char *names, const char *desc, int64_t default_value, 
                                 uint32_t opts, int64_t range_min, int64_t range_max)
{
    assert((range_min <= range_max) && (default_value >= range_min) && (default_value <= range_max));
    return (int64_t *) &hgl_flag_create_(HGL_FLAG_KIND_I64, names, desc, transmute(uintptr_t, default_value), 
                                         opts, transmute(uintptr_t, range_min), transmute(uintptr_t, range_max))->value;
}

uint64_t *hgl_flags_add_u64(const char *names, const char *desc, uint64_t default_value, uint32_t opts)
{
    return (uint64_t *) &hgl_flag_create_(HGL_FLAG_KIND_U64, names, desc, transmute(uintptr_t, default_value), 
                                          opts, 0, ULONG_MAX)->value;
}

uint64_t *hgl_flags_add_u64_range(const char *names, const char *desc, uint64_t default_value, 
                                  uint32_t opts, uint64_t range_min, uint64_t range_max)
{
    assert((range_min <= range_max) && (default_value >= range_min) && (default_value <= range_max));
    return (uint64_t *) &hgl_flag_create_(HGL_FLAG_KIND_U64, names, desc, transmute(uintptr_t, default_value), 
                                          opts, transmute(uintptr_t, range_min), transmute(uintptr_t, range_max))->value;
}

double *hgl_flags_add_f64(const char *names, const char *desc, double default_value, uint32_t opts)
{
    double f64_max =  DBL_MAX;
    double f64_min = -DBL_MAX;
    return (double *) &hgl_flag_create_(HGL_FLAG_KIND_F64, names, desc, transmute(uintptr_t, default_value), 
                                        opts, transmute(uintptr_t, f64_min), transmute(uintptr_t, f64_max))->value;
} 

double *hgl_flags_add_f64_range(const char *names, const char *desc, double default_value, 
                                uint32_t opts, double range_min, double range_max)
{
    assert((range_min <= range_max) && (default_value >= range_min) && (default_value <= range_max));
    return (double *) &hgl_flag_create_(HGL_FLAG_KIND_F64, names, desc, transmute(uintptr_t, default_value), 
                                        opts, transmute(uintptr_t, range_min), transmute(uintptr_t, range_max))->value;
}

const char **hgl_flags_add_str(const char *names, const char *desc, const char *default_value, uint32_t opts)
{
    return (const char **) &hgl_flag_create_(HGL_FLAG_KIND_STR, names, desc, transmute(uintptr_t, default_value), opts, 0, 0)->value;
} 
 
static inline bool is_delimiting_char_(char c)
{
    return (c == '\n') ||
           (c == '\t') ||
           (c == ' ')  ||
           (c == '\r') ||
           (c == ',')  ||
           (c == '\0');
}

int hgl_flags_parse(int argc, char *argv[])
{
    /* iterate argument list */
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        size_t arg_len = strlen(arg);

        /* iterate flags */
        bool match = false;
        for (size_t j = 0; (j < hgl_n_flags_) && !match; j++) {
            /* iterate flag names */
            const char *names = hgl_flags_[j].names;
            const char *name  = names;

            size_t names_len  = strlen(names);
            size_t name_len   = 0;

            while ((name + name_len) < (names + names_len)) {

                /* find next `name` */
                name_len = 0;
                while (!is_delimiting_char_(name[name_len])) name_len++;

                /* compare `name` with arg string */
                if ((name_len == arg_len) && (strncmp(name, arg, name_len) == 0)) {
                    match = true;
                    break;
                }

                /* go to possible next `name` in `names` */
                name += name_len + 1;
                continue;
            }
            
            if (!match) {
                continue;
            }
        
            /* if the option takes an argument check that argv[i + 1] exists. */
            char *next_arg;
            char *end;
            HglFlagKind kind = hgl_flags_[j].kind;
            if ((kind == HGL_FLAG_KIND_I64) || (kind == HGL_FLAG_KIND_U64) ||
                (kind == HGL_FLAG_KIND_F64) || (kind == HGL_FLAG_KIND_STR)) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Error parsing option: Option `%s` takes "
                            "an argument. User provided nothing.\n", names);
                    return -1;
                }

                /* read next early */
                next_arg = argv[++i];
            }

            switch (kind) {

                /* parse simple boolean option flag */
                case HGL_FLAG_KIND_BOOL: {
                    bool truev = true;
                    hgl_flags_[j].value = transmute(uintptr_t, truev);
                } break;

                /* parse i64 flag */
                case HGL_FLAG_KIND_I64: {
                    int64_t value = strtol(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "an int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* clamp to range */
                    int64_t range_min = transmute(int64_t, hgl_flags_[j].range_min); 
                    int64_t range_max = transmute(int64_t, hgl_flags_[j].range_max); 
                    int64_t old_value = value;
                    value = min(max(old_value, range_min), range_max);

                    hgl_flags_[j].status |= (value > old_value) ? HGL_FLAG_STATUS_RANGE_UNDERFLOW : 0;
                    hgl_flags_[j].status |= (value < old_value) ? HGL_FLAG_STATUS_RANGE_OVERFLOW : 0;
                    hgl_flags_[j].value = transmute(uintptr_t, value);
                } break;
                
                case HGL_FLAG_KIND_U64: {
                    uint64_t value = strtoul(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "an int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* clamp to range */
                    uint64_t range_min = transmute(uint64_t, hgl_flags_[j].range_min); 
                    uint64_t range_max = transmute(uint64_t, hgl_flags_[j].range_max); 
                    uint64_t old_value = value;
                    value = min(max(old_value, range_min), range_max);

                    hgl_flags_[j].status |= (value > old_value) ? HGL_FLAG_STATUS_RANGE_UNDERFLOW : 0;
                    hgl_flags_[j].status |= (value < old_value) ? HGL_FLAG_STATUS_RANGE_OVERFLOW : 0;
                    hgl_flags_[j].value = transmute(uintptr_t, value);
                } break;

                /* parse float64 flag */
                case HGL_FLAG_KIND_F64: {
                    double value = strtod(next_arg, &end);
                    
                    /* Check if strtof failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "a float. User provided: %s\n", names, next_arg);
                        return -1;
                    }
                    
                    /* clamp to range */
                    double range_min = transmute(double, hgl_flags_[j].range_min); 
                    double range_max = transmute(double, hgl_flags_[j].range_max); 
                    double old_value = value;
                    value = fmin(fmax(old_value, range_min), range_max);

                    hgl_flags_[j].status |= (value > old_value) ? HGL_FLAG_STATUS_RANGE_UNDERFLOW : 0;
                    hgl_flags_[j].status |= (value < old_value) ? HGL_FLAG_STATUS_RANGE_OVERFLOW : 0;
                    hgl_flags_[j].value = transmute(uintptr_t, value);
                } break;
                
                /* parse string flag */
                case HGL_FLAG_KIND_STR: {
                    hgl_flags_[j].value = transmute(uintptr_t, next_arg);
                } break;
            }

            /* mark flag as parsed */
            hgl_flags_[j].status = hgl_flags_[j].status | HGL_FLAG_STATUS_PARSED;
        }
            
        if (!match) {
            fprintf(stderr, "Invalid option: \"%s\"\n", arg);
            return -1; 
        }
    }
   
    int err = 0;

    /* assert that mandatory flags have been parsed */
    for (size_t i = 0; i < hgl_n_flags_; i++) {
        HglFlag flag = hgl_flags_[i];
        if (((flag.opts & HGL_FLAG_OPT_MANDATORY) != 0) &&
            ((flag.status & HGL_FLAG_STATUS_PARSED) == 0)) {
            fprintf(stderr, "Error: Option marked as mandatory not provided: `%s`\n", flag.names);
            err = 1;
        }
        if (flag.status & (HGL_FLAG_STATUS_RANGE_OVERFLOW | HGL_FLAG_STATUS_RANGE_UNDERFLOW)) {
            fprintf(stderr, "Warning: Option `%s` was provided with an out-of-range value. "
                    "Value has been clamped to: ", flag.names);
            switch (flag.kind) {
                case HGL_FLAG_KIND_I64: {
                    fprintf(stderr, "%ld. Valid range = [%ld, %ld]\n", transmute(int64_t, flag.value), 
                            transmute(int64_t, flag.range_min), transmute(int64_t, flag.range_max));
                } break;
                case HGL_FLAG_KIND_U64: {
                    fprintf(stderr, "%lu. Valid range = [%lu, %lu]\n", transmute(uint64_t, flag.value), 
                            transmute(uint64_t, flag.range_min), transmute(uint64_t, flag.range_max));
                } break;
                case HGL_FLAG_KIND_F64: {
                    fprintf(stderr, "%.9g. Valid range = [%.9g, %.9g]\n", (double) transmute(double, flag.value), 
                            (double) transmute(double, flag.range_min), (double) transmute(double, flag.range_max));
                } break;
                default: assert(0 && "Unreachable"); break;
            }
        }
    }

    return err;
}

void hgl_flags_print()
{
    printf("Options:\n");
    for (size_t i = 0; i < hgl_n_flags_; i++) {
        HglFlagKind kind        = hgl_flags_[i].kind;
        const char *names       = hgl_flags_[i].names;
        const char *desc        = hgl_flags_[i].desc;
        uintptr_t default_value = hgl_flags_[i].default_value;
        uintptr_t range_min     = hgl_flags_[i].range_min;
        uintptr_t range_max     = hgl_flags_[i].range_max;
        switch (kind) {
            case HGL_FLAG_KIND_BOOL:        printf("  %-24s %s (default = %d)\n", names, desc, transmute(bool, default_value)); break;
            case HGL_FLAG_KIND_I64: {
                printf("  %-24s %s (default = %ld, valid range = [%ld, %ld])\n", names, desc, 
                       transmute(int64_t, default_value), transmute(int64_t, range_min), transmute(int64_t, range_max));
            } break;
            case HGL_FLAG_KIND_U64: {
                printf("  %-24s %s (default = %lu, valid range = [%lu, %lu])\n", names, desc, 
                       transmute(uint64_t, default_value), transmute(uint64_t, range_min), transmute(uint64_t, range_max));
            } break;
            case HGL_FLAG_KIND_F64: { //       printf("  %-24s %s (default = %f)\n", names, desc, transmute(float, default_value)); break;
                printf("  %-24s %s (default = %.9g, valid range = [%.9g, %.9g])\n", names, desc, 
                       transmute(double, default_value), transmute(double, range_min), transmute(double, range_max));
            } break;
            case HGL_FLAG_KIND_STR:         printf("  %-24s %s (default = %s)\n", names, desc, transmute(char *, default_value)); break;
        }
    }
}

#endif

