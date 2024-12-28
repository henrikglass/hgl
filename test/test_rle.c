
#include "hgl_test.h"

#define HGL_RLE_IMPLEMENTATION
#include "hgl_rle.h"

#define N 1024*1024

uint8_t buf1[N];
uint8_t buf2[N];

TEST(encode_decode8_8)
{
    const char *data = "Heeeeeeeeeejjjsannn hoppsaaan! :)";
    size_t original_size = strlen(data) + 1;
    size_t encoded_size = hgl_rle_encode8_8(buf1, (uint8_t *)data, original_size);
    size_t decoded_size = hgl_rle_decode8_8(buf2, buf1, encoded_size);
    ASSERT(encoded_size > 0);
    ASSERT(decoded_size == original_size);
    ASSERT(memcmp(data, buf2, original_size) == 0);
}

TEST(encode_decode32_32)
{
    static uint32_t data[] = {
        0x0000000A, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0xFF00FFFF, 0xFF00FFFF, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000B000, 0x00000000,
    };
    size_t original_size = sizeof(data);
    size_t encoded_size = hgl_rle_encode32_32((uint32_t *)buf1, data, original_size);
    size_t decoded_size = hgl_rle_decode32_32((uint32_t *)buf2, (uint32_t *)buf1, encoded_size);
    ASSERT(encoded_size > 0);
    ASSERT(decoded_size == original_size);
    ASSERT(memcmp(data, buf2, original_size) == 0);
}

TEST(encode_decode32_8)
{
    static uint32_t data[] = {
        0x0000000A, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0xFF00FFFF, 0xFF00FFFF, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000A00,
    };
    size_t original_size = sizeof(data);
    size_t encoded_size = hgl_rle_encode32_8(buf1, data, original_size);
    size_t decoded_size = hgl_rle_decode32_8((uint32_t *)buf2, buf1, encoded_size);
    ASSERT(encoded_size > 0);
    ASSERT(decoded_size == original_size);
    ASSERT(memcmp(data, buf2, original_size) == 0);
}
