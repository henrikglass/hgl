
#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#define HGL_HAMMING_IMPLEMENTATION
#include "hgl_hamming.h"

int main(int argc, char *argv[])
{
    (void) argc;

    const char *infile_path = argv[1];
    const char *outfile_path = "encoded.hm";

    HglFile file = hgl_io_read_file(infile_path);

    /* prepare input buffer */
    size_t data_size = HGL_HAMMING_ROUND_UP(file.size, 11);
    uint8_t *data = calloc(1, data_size);
    memcpy(data, file.data, file.size);
    hgl_io_free_file(&file);

    /* prepare output buffer */
    size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(data_size);
    HglHamming16_11 *encoded = calloc(1, encoded_size);

    /* encode data */
    printf("original size = %zu\n", data_size);
    printf("encoded size = %zu\n", encoded_size);
    int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, data_size);
    if (err != 0) {
        return 1;
    }

    /*
     * write encoded data to file.
     *
     * Yes, I know this has different behavior depending on endianness. I don't care.
     */
    hgl_io_write_to_filepath(outfile_path, (uint8_t *) encoded, encoded_size);

    /* cleanup */
    free(encoded);
}
