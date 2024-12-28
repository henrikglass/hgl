
#include "hgl_test.h"

#define HGL_HAMMING_DEBUG_PRINTS
#define HGL_HAMMING_IMPLEMENTATION
#include "hgl_hamming.h"

TEST(test_encode_decode, .expect_output = "Hello World!\n")
{
    uint8_t data[22] = "Hello World!";
    size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(sizeof(data));
    size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(encoded_size);
    HglHamming16_11 *encoded = malloc(encoded_size);
    uint8_t *decoded = malloc(encoded_size);
    ASSERT(encoded != NULL);
    ASSERT(decoded != NULL);

    int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, sizeof(data));
    ASSERT(err == 0);

    err = hgl_hamming_decode_16_11(decoded, decoded_size, encoded, encoded_size);
    ASSERT(err == 0);

    printf("%s\n", decoded);
}

TEST(test_encode_decode_single_bit_errors, .expect_output = "Hello World!\n")
{
    uint8_t data[22] = "Hello World!";
    size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(sizeof(data));
    size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(encoded_size);
    HglHamming16_11 *encoded = malloc(encoded_size);
    uint8_t *decoded = malloc(encoded_size);
    ASSERT(encoded != NULL);
    ASSERT(decoded != NULL);

    int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, sizeof(data));
    ASSERT(err == 0);

    encoded[0] ^= 0x0200;
    encoded[1] ^= 0x4000;
    encoded[2] ^= 0x2000;
    encoded[3] ^= 0x0002;
    encoded[4] ^= 0x0040;

    err = hgl_hamming_decode_16_11(decoded, decoded_size, encoded, encoded_size);
    ASSERT(err == 5);

    printf("%s\n", decoded);
}

TEST(test_encode_decode_double_bit_errors)
{
    uint8_t data[22] = "Hello World!";
    size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(sizeof(data));
    size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(encoded_size);
    HglHamming16_11 *encoded = malloc(encoded_size);
    uint8_t *decoded = malloc(encoded_size);
    ASSERT(encoded != NULL);
    ASSERT(decoded != NULL);

    int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, sizeof(data));
    ASSERT(err == 0);

    encoded[1] ^= 0x4010;
    encoded[4] ^= 0x0140;

    err = hgl_hamming_decode_16_11(decoded, decoded_size, encoded, encoded_size);
    ASSERT(err == -2);

    LOG("%s\n", decoded);
}
