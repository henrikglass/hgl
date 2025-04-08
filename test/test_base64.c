#include "hgl_test.h"

#define HGL_BASE64_IMPLEMENTATION
#include "hgl_base64.h"

TEST(encode_decode)
{
    const char *str = "Hejsan hoppsan hallå! :>";
    size_t original_size = strlen(str) + 1;

    size_t encoded_size = hgl_base64_encode(NULL, NULL, original_size);
    char *encoded = malloc(encoded_size);
    size_t encoded_size_2 = hgl_base64_encode(encoded, str, original_size);

    size_t decoded_size = hgl_base64_decode(NULL, encoded, encoded_size);
    char *decoded = malloc(decoded_size);
    size_t decoded_size2 = hgl_base64_decode(decoded, encoded, encoded_size);

    assert(encoded_size == encoded_size_2);
    assert(decoded_size == decoded_size2);
    assert(decoded_size == original_size);
    assert(strcmp(str, decoded) == 0);
}
