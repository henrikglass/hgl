#define _DEFAULT_SOURCE

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

#define HGL_RLE_IMPLEMENTATION
#include "hgl_rle.h"

#define SCRATCH_BUF_SIZE 1l*1024l*1024l*1024l // 1 GiB
uint8_t scratch[SCRATCH_BUF_SIZE];

typedef enum
{
    BLOCK_8_8,
    BLOCK_32_8,
    BLOCK_32_32,
} CompressionKind;

int main(int argc, char *argv[])
{
    /* parse opts */
    bool *opt_deflate = hgl_flags_add_bool("--deflate", "deflate file", false, 0);
    bool *opt_inflate = hgl_flags_add_bool("--inflate", "inflate file", false, 0);
    const char **opt_input  = hgl_flags_add_str("-i,--input", "input file path", NULL, HGL_FLAGS_OPT_MANDATORY);
    const char **opt_output = hgl_flags_add_str("-o,--output", "output file path", NULL, 0);
    bool *opt_8_8   = hgl_flags_add_bool("--8_8", "----", false, 0);
    bool *opt_32_8  = hgl_flags_add_bool("--32_8", "----", false, 0);
    bool *opt_32_32 = hgl_flags_add_bool("--32_32", "----", false, 0);
    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }

    if (!(*opt_deflate ^ *opt_inflate)) {
        printf("Error: Either `--deflate` or `--inflate` must be chosen.\n");
    }

    if (!(*opt_8_8 ^ *opt_32_8 ^ *opt_32_32)) {
        printf("Error: Either `--8_8`, `--32_8`, or `--32_32` must be chosen.\n");
    }
 
    CompressionKind kind = *opt_8_8   ? BLOCK_8_8   :
                           *opt_32_8  ? BLOCK_32_8  :
                           *opt_32_32 ? BLOCK_32_32 :
                                        BLOCK_8_8;
    /* open input file */
    HglFile input_file = hgl_io_file_read(*opt_input);
    ssize_t deflated_size;
    ssize_t inflated_size;

    if (*opt_deflate) {
        /* create output filename if none provided */
        if (*opt_output == NULL) {
            HglStringBuilder sb = hgl_sb_make(*opt_input, 0); 
            hgl_sb_append_cstr(&sb, ".hz");
            *opt_output = sb.cstr;
        }

        /* compute encoded size */
        switch (kind) {
            case BLOCK_8_8:   deflated_size = hgl_rle_encode8_8(NULL, input_file.data, input_file.size); break;
            case BLOCK_32_8:  deflated_size = hgl_rle_encode32_8(NULL, (uint32_t *)input_file.data, input_file.size); break;
            case BLOCK_32_32: deflated_size = hgl_rle_encode32_32(NULL, (uint32_t *)input_file.data, input_file.size); break;
        }

        if (deflated_size == -1) {
            return 1;
        }

        if (deflated_size > SCRATCH_BUF_SIZE) {
            printf("Encoded size too large.\n");
            return 1;
        }

        /* deflate data */
        switch (kind) {
            case BLOCK_8_8:   hgl_rle_encode8_8(scratch, input_file.data, input_file.size); break;
            case BLOCK_32_8:  hgl_rle_encode32_8(scratch, (uint32_t *)input_file.data, input_file.size); break;
            case BLOCK_32_32: hgl_rle_encode32_32((uint32_t *)scratch, (uint32_t *)input_file.data, input_file.size); break;
        }

        /* 32_32 compression? convert to known endian */
        if (kind == BLOCK_32_32) {
            for (size_t i = 0; i < (size_t)deflated_size / 4; i++) {
                ((uint32_t*)scratch)[i] = htobe32(((uint32_t*)scratch)[i]);
            }
        }

        /* write file */
        err = hgl_io_file_write_to(*opt_output, scratch, deflated_size);
        if (err != 0) {
            return 1;
        }

    } else if (*opt_inflate) {
        /* create output filename if none provided */
        if (*opt_output == NULL) {
            HglStringBuilder sb = hgl_sb_make(*opt_input, 0); 
            hgl_sb_append_cstr(&sb, ".deflated");
            *opt_output = sb.cstr;
        }

        /* 32_32 compression? convert to native endian */
        if (kind == BLOCK_32_32) {
            for (size_t i = 0; i < input_file.size / 4; i++) {
                ((uint32_t *)input_file.data)[i] = be32toh(((uint32_t *)input_file.data)[i]);
            }
        }

        /* compute decoded size */
        switch (kind) {
            case BLOCK_8_8:   inflated_size = hgl_rle_decode8_8(NULL, input_file.data, input_file.size); break;
            case BLOCK_32_8:  inflated_size = hgl_rle_decode32_8(NULL, input_file.data, input_file.size); break;
            case BLOCK_32_32: inflated_size = hgl_rle_decode32_32(NULL, (uint32_t *) input_file.data, input_file.size); break;
        }

        if (inflated_size == -1) {
            return 1;
        }
        if (inflated_size > SCRATCH_BUF_SIZE) {
            printf("Decoded size too large.\n");
            return 1;
        }

        /* inflate data */
        switch (kind) {
            case BLOCK_8_8:   hgl_rle_decode8_8(scratch, input_file.data, input_file.size); break;
            case BLOCK_32_8:  hgl_rle_decode32_8((uint32_t *)scratch, input_file.data, input_file.size); break;
            case BLOCK_32_32: hgl_rle_decode32_32((uint32_t *)scratch, (uint32_t *) input_file.data, input_file.size); break;
        }

        /* write file */
        err = hgl_io_file_write_to(*opt_output, scratch, inflated_size);
        if (err != 0) {
            return 1;
        }
    } else {
        assert(0 && "Unreachable.");
    }


    return 0;
}

