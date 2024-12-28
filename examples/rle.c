
#define HGL_RLE_IMPLEMENTATION
#include "hgl_rle.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static char buf1[160000000];
static char buf2[160000000];

static uint32_t data[] = {
    0x0000000A, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF00FFFF, 0xFF00FFFF, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

int main()
{
#if 1
    printf("hello world!\n");

    static const char *str = "Heeeeeeeejjsaaaaaaaaaaaaaaaaaaaaaaaan hoppsaaan";

    size_t size = strlen(str) + 1;
    size_t encoded_size = hgl_rle_encode8_8((uint8_t *)buf1, (const uint8_t *)str, size);
    size_t decoded_size = hgl_rle_decode8_8((uint8_t *)buf2, (const uint8_t *)buf1, encoded_size);

    printf("original size = %zu\n", size);
    printf("compressed size = %zu\n", encoded_size);
    printf("uncompressed size = %zu\n", decoded_size);

    printf("original string: \"%s\"\n", str);
    printf("decoded string: \"%s\"\n", buf2);

    assert(size == decoded_size);
    assert(strcmp(str, buf2) == 0);
#endif

#if 1
    encoded_size = hgl_rle_encode32_32((uint32_t *)buf1, data, sizeof(data));
    decoded_size = hgl_rle_decode32_32((uint32_t *)buf2, (uint32_t *)buf1, encoded_size);

    printf("original size = %zu\n", sizeof(data));
    printf("compressed size = %zu\n", encoded_size);
    printf("uncompressed size = %zu\n", decoded_size);

    assert(sizeof(data) == decoded_size);
    assert(memcmp(buf2, data, decoded_size) == 0);
    printf("%08X \n", ((uint32_t *)buf2)[25]);
    printf("%08X \n", ((uint32_t *)buf2)[26]);
    printf("%08X \n", ((uint32_t *)buf2)[27]);
    printf("%08X \n", ((uint32_t *)buf2)[28]);
    printf("%08X \n", ((uint32_t *)buf2)[29]);
#endif

#if 1
    encoded_size = hgl_rle_encode32_8((uint8_t *)buf1, data, sizeof(data));
    decoded_size = hgl_rle_decode32_8((uint32_t *)buf2, (uint8_t *)buf1, encoded_size);

    printf("original size = %zu\n", sizeof(data));
    printf("compressed size = %zu\n", encoded_size);
    printf("uncompressed size = %zu\n", decoded_size);

    assert(sizeof(data) == decoded_size);
    assert(memcmp(buf2, data, decoded_size) == 0);
    printf("%08X \n", ((uint32_t *)buf2)[25]);
    printf("%08X \n", ((uint32_t *)buf2)[26]);
    printf("%08X \n", ((uint32_t *)buf2)[27]);
    printf("%08X \n", ((uint32_t *)buf2)[28]);
    printf("%08X \n", ((uint32_t *)buf2)[29]);
#endif

#if 1
    printf("\n------------------------------------------\n");
    size = 10000000*sizeof(uint32_t);
    uint32_t *data = malloc(size);
    assert(data != NULL);
    data[1245] = 4322123;
    data[12] = 1235;
    data[100020] = 4412;
    data[121230] = 432;
    data[400000] = 123111;
    encoded_size = hgl_rle_encode32_32((uint32_t *)buf1, data, size);
    decoded_size = hgl_rle_decode32_32((uint32_t *)buf2, (uint32_t *)buf1, encoded_size);
    printf("og size = %zu\n", size);
    printf("compressed size = %zu\n", encoded_size);
    printf("uncompressed size = %zu\n", decoded_size);
    assert(10000000*sizeof(uint32_t) == decoded_size);
    assert(memcmp(buf2, data, decoded_size) == 0);
#endif
}

