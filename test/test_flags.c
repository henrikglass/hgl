#include <stdio.h>

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

int main(int argc, char *argv[])
{

    bool *a          = hgl_flags_add_bool("-a", "option a", false, 0);
    bool *b          = hgl_flags_add_bool("-b,--bee", "option b", false, 0);
    int *c           = hgl_flags_add_int("-c", "int c", 123456, HGL_FLAG_OPT_MANDATORY);
    long *d          = hgl_flags_add_long("-d", "looong d", 0xFF, 0);
    unsigned long *u = hgl_flags_add_ulong("-u", "uulooong d", 0xFFFFF0FFFFFFFFFF, 0);
    float *f         = hgl_flags_add_float("-f,--float", "float f hello", 3.1415926535f, 0);
    const char **o   = hgl_flags_add_str("-o,--output", "output file path", "a.out", 0);

    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }
    
    printf("a = %d, b = %d, c = %d, d = %ld, u = %lu, f = %f, o = %s\n", *a, *b, *c, *d, *u, (double) *f, *o);

}
