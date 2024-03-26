#include <stdio.h>

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

int main(int argc, char *argv[])
{

    bool *a          = hgl_flags_add_bool("-a", "option a", false, 0);
    bool *b          = hgl_flags_add_bool("-b,--bee", "option b", false, HGL_FLAG_OPT_MANDATORY);
    int64_t *c       = hgl_flags_add_i64("-c", "int c", 123456, 0);
    int64_t *d       = hgl_flags_add_i64("-d", "looong d", 0xFF, 0);
    uint64_t *u      = hgl_flags_add_u64("-u", "uulooong d", 0xFFFFF0FFFFFFFFFF, 0);
    double *f        = hgl_flags_add_f64_range("-f,--float", "float f hello", 103.1415926535f, 0, 10.0, DBL_MAX);
    const char **o   = hgl_flags_add_str("-o,--output", "output file path", "a.out", 0);

    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }
    
    printf("a = %d, b = %d, c = %ld, d = %ld, u = %lu, f = %f, o = %s\n", *a, *b, *c, *d, *u, *f, *o);

}
