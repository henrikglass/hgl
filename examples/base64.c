
#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

#define HGL_BASE64_IMPLEMENTATION
#include "hgl_base64.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SCRATCH_BUF_SIZE (16 * 1024 * 1024)

int main(int argc, char *argv[])
{
    /* Parse args */
    bool *opt_decode = hgl_flags_add_bool("-d,--decode", "Decode base64-encoded input on stdin", false, 0);
    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }

    /* Encode/Decode input */
    static uint8_t input[SCRATCH_BUF_SIZE] = {0};
    static uint8_t output[SCRATCH_BUF_SIZE] = {0};
    size_t n_read_bytes = read(0, input, SCRATCH_BUF_SIZE);

    if (n_read_bytes <= 0) {
        return 1;
    }

    if (*opt_decode) {
        hgl_base64_decode(output, input, 4 * (n_read_bytes / 4));
        printf("%s", output);
    } else {
        hgl_base64_encode(output, input, n_read_bytes);
        printf("%s\n", output);
    }

    fflush(stdout);
}
