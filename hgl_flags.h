
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

#ifndef HGL_FLAG_MAX_N_FLAGS
#define HGL_FLAG_MAX_N_FLAGS 32
#endif
#define HGL_FLAG_OPT_MANDATORY 0x00000001u

typedef enum
{
    HGL_FLAG_KIND_BOOL,
    HGL_FLAG_KIND_INT,
    HGL_FLAG_KIND_LONG,
    HGL_FLAG_KIND_ULONG,
    HGL_FLAG_KIND_FLOAT,
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
} HglFlag;

/**
 * Add a flag of type `bool`.
 */
bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts);

/**
 * Add a flag of type `int`.
 */
int *hgl_flags_add_int(const char *names, const char *desc, int default_value, uint32_t opts);

/**
 * Add a flag of type `long`.
 */
long *hgl_flags_add_long(const char *names, const char *desc, long default_value, uint32_t opts);

/**
 * Add a flag of type `unsigned long`.
 */
unsigned long *hgl_flags_add_ulong(const char *names, const char *desc, unsigned long default_value, uint32_t opts);

/**
 * Add a flag of type `float`.
 */
float *hgl_flags_add_float(const char *names, const char *desc, float default_value, uint32_t opts);

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

#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)
#define transmute(T, var) (*(T *) &(var))
#define HGL_FLAG_STATUS_PARSED 0x00000001u

static HglFlag hgl_flags_[HGL_FLAG_MAX_N_FLAGS] = {0};
static size_t hgl_n_flags_ = 0;

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc, 
                         uintptr_t default_value, uint32_t opts);

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc, 
                          uintptr_t default_value, uint32_t opts)
{
    hgl_flags_[hgl_n_flags_++] = (HglFlag) {
        .kind          = kind,
        .names         = names,
        .default_value = default_value,
        .value         = default_value,
        .opts          = opts,
        .desc          = desc,
        .status        = 0,
    }; 
    return &hgl_flags_[hgl_n_flags_ - 1];
}

bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts)
{
    return (bool *) &hgl_flag_create_(HGL_FLAG_KIND_BOOL, names, desc, transmute(uintptr_t, default_value), opts)->value;
} 

int *hgl_flags_add_int(const char *names, const char *desc, int default_value, uint32_t opts)
{
    return (int *) &hgl_flag_create_(HGL_FLAG_KIND_INT, names, desc, transmute(uintptr_t, default_value), opts)->value;
} 

long *hgl_flags_add_long(const char *names, const char *desc, long default_value, uint32_t opts)
{
    return (long *) &hgl_flag_create_(HGL_FLAG_KIND_LONG, names, desc, transmute(uintptr_t, default_value), opts)->value;
} 

unsigned long *hgl_flags_add_ulong(const char *names, const char *desc, unsigned long default_value, uint32_t opts)
{
    return (unsigned long *) &hgl_flag_create_(HGL_FLAG_KIND_ULONG, names, desc, transmute(uintptr_t, default_value), opts)->value;
} 

float *hgl_flags_add_float(const char *names, const char *desc, float default_value, uint32_t opts)
{
    return (float *) &hgl_flag_create_(HGL_FLAG_KIND_FLOAT, names, desc, transmute(uintptr_t, default_value), opts)->value;
} 

const char **hgl_flags_add_str(const char *names, const char *desc, const char *default_value, uint32_t opts)
{
    return (const char **) &hgl_flag_create_(HGL_FLAG_KIND_STR, names, desc, transmute(uintptr_t, default_value), opts)->value;
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
            if ((kind == HGL_FLAG_KIND_INT) || (kind == HGL_FLAG_KIND_LONG) ||
                (kind == HGL_FLAG_KIND_FLOAT) || (kind == HGL_FLAG_KIND_STR) ||
                (kind == HGL_FLAG_KIND_ULONG)) {
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
                    hgl_flags_[j].value = (uintptr_t) true;     
                } break;

                /* parse int flag */
                case HGL_FLAG_KIND_INT:
                case HGL_FLAG_KIND_LONG: {
                    long value = strtol(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "an int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* int: clamp to range [INT_MIN, INT_MAX] */
                    if (kind == HGL_FLAG_KIND_INT) {
                        value = min(max(value, INT_MIN), INT_MAX);
                    }

                    hgl_flags_[j].value = transmute(uintptr_t, value);
                } break;
                
                case HGL_FLAG_KIND_ULONG: {
                    long value = strtoul(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "an int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    hgl_flags_[j].value = transmute(uintptr_t, value);
                } break;

                /* parse float flag */
                case HGL_FLAG_KIND_FLOAT: {
                    float value = strtof(next_arg, &end);
                    
                    /* Check if strtof failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, "Error parsing option: Option `%s` takes "
                                "a float. User provided: %s\n", names, next_arg);
                        return -1;
                    }
                    
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
        if (((hgl_flags_[i].opts & HGL_FLAG_OPT_MANDATORY) != 0) &&
            ((hgl_flags_[i].status & HGL_FLAG_STATUS_PARSED) == 0)) {
            fprintf(stderr, "Option marked as mandatory not provided: `%s`\n", hgl_flags_[i].names);
            err = 1;
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
        switch (kind) {
            case HGL_FLAG_KIND_BOOL:  printf("  %-24s %s (default = %d)\n", names, desc, *(bool*) &default_value); break;
            case HGL_FLAG_KIND_INT:   printf("  %-24s %s (default = %d)\n", names, desc, *(int*) &default_value); break;
            case HGL_FLAG_KIND_LONG:  printf("  %-24s %s (default = %ld)\n", names, desc, *(long*) &default_value); break;
            case HGL_FLAG_KIND_ULONG: printf("  %-24s %s (default = %lu)\n", names, desc, *(unsigned long*) &default_value); break;
            case HGL_FLAG_KIND_FLOAT: printf("  %-24s %s (default = %f)\n", names, desc, *(float*) &default_value); break;
            case HGL_FLAG_KIND_STR:   printf("  %-24s %s (default = %s)\n", names, desc, *(char**) &default_value); break;
        }
    }
}

#endif

