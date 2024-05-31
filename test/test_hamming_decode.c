
#define HGL_IO_IMPLEMENTATION
#include "hgl_io.h"

#define HGL_HAMMING_IMPLEMENTATION
#include "hgl_hamming.h"

int main(int argc, char *argv[])
{
    (void) argc;

    const char *infile_path = argv[1];

    HglFile file = hgl_io_read_file(infile_path);
    assert(file.size % 16 == 0);
    assert(file.data != NULL);

    /* prepare output buffer */
    size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(file.size);
    uint8_t *decoded = calloc(1, decoded_size);

    /*
     * encode data.
     *
     * Something, something, endianness...
     */
    int ret = hgl_hamming_decode_16_11(decoded, decoded_size, (HglHamming16_11 *) file.data, file.size);
    if (ret < 0) {
        return 1;
    }

    /* print contents */
    printf("decoded data = \"%s\"\n", decoded);

    /* cleanup */
    free(decoded);
}
