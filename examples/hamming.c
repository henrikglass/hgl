
#define HGL_HAMMING_IMPLEMENTATION
#include "hgl_hamming.h"

#include <stdlib.h>

int main()
{

    /* =========== ENCODE =========== */

    // message to encode
    uint8_t data[110] = "Hello world! :)";

    // allocate memory for encoded message
    size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(sizeof(data));
    HglHamming16_11 *encoded = malloc(encoded_size);

    // encode message
    int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, sizeof(data));
    if (err != 0) {
        printf("Encode error: Invalid argument(s).\n");
        return 1;
    }

    /* ==== DAMAGE ENCODED DATA ===== */

    encoded[0] ^= 0x0001;
    encoded[1] ^= 0x0800;
    encoded[2] ^= 0x0010;
    encoded[3] ^= 0x4000;
    encoded[4] ^= 0x0001;

    // flipping the block parity bit should be fine if the rest of the block
    // has no parity errors.
    encoded[5] ^= 0x8000;

    /* =========== DECODE =========== */

    // allocate memory for decoded message
    size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(encoded_size);
    uint8_t *decoded = malloc(decoded_size);

    // decode message
    int ret = hgl_hamming_decode_16_11(decoded, decoded_size, encoded, encoded_size);
    if (ret < 0) {
        if (errno == EINVAL) {
            printf("Decode error: Invalid argument(s).\n");
        } else {
            printf("Decode error: Found %d corrupted blocks.\n", -ret);
        }
        return 1;
    }

    printf("Decode succeeded. %d/%zu bits needed correcting.\n", ret, 8*decoded_size);

    /* ============================== */

    printf("Decoded message = \"%s\"\n", decoded);

    free(encoded);
    free(decoded);
    return 0;
}
