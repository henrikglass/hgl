/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_base64.h implements base64 encoding & decoding.
 *
 *
 * USAGE:
 *
 * Include hgl_base64.h like this:
 *
 *     #define HGL_BASE64_IMPLEMENTATION
 *     #include <hgl_base64.h>
 *
 * HGL_BASE64_IMPLEMENTATION must only be defined once, in a single compilation unit.
 *
 *
 * EXAMPLE:
 *
 *     const char *str = "Hello World";
 *     size_t size = strlen(str) + 1;
 *
 *     // Encode
 *     size_t encoded_size = hgl_base64_encode(NULL, NULL, size);
 *     const uint8_t *buf = malloc(encoded_size);
 *     hgl_base64_encode(buf, str, size);
 *
 *     // Decode
 *     size_t decoded_size = hgl_base64_decode(NULL, buf, encoded_size);
 *     const uint8_t *buf2 = malloc(decoded_size);
 *     hgl_base64_decode(buf2, buf, encoded_size);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_BASE64_H
#define HGL_BASE64_H

#include <stdint.h>
#include <stddef.h>

/**
 * If `dst` is NULL, hgl_base64_encode calculates and returns the encoded
 * data size given `src_size`.
 *
 * If `dst` is not NULL, hgl_base64_encode reads the data at `src`, encodes
 * it, and places it into `dst`.
 */
size_t hgl_base64_encode(void *dst, const void *src, size_t src_size);

/**
 * If `dst` is NULL, hgl_base64_decode calculates and returns the decoded
 * data size given `src` and `src_size`.
 *
 * If both `dst` and `src` is NULL, hgl_base64_decode returns the upper
 * bound of the decoded data size given `src_size`.
 *
 * If `dst` is not NULL, hgl_base64_decodes reads the data at `src`, decodes
 * it, and places it into `dst`.
 */
size_t hgl_base64_decode(void *dst, const void *src, size_t src_size);

#endif /* HGL_BASE64_H */

#ifdef HGL_BASE64_IMPLEMENTATION

#include <assert.h>

size_t hgl_base64_encode(void *dst, const void *src, size_t src_size)
{
    uint8_t *dst8 = (uint8_t *) dst;
    const uint8_t *src8 = (const uint8_t *) src;
    size_t src_idx = 0;
    size_t dst_idx = 0;
    size_t encoded_size = 4*(((src_size + 2) / 3));

    /* dst == NULL? Just return encoded size */
    if (dst == NULL) {
        return encoded_size;
    }

    size_t n_blocks = encoded_size / 4;
    for (size_t i = 0; i < n_blocks; i++) {
        uint8_t block[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        for (size_t j = 0; j < 3; j++) {
            if ((src_idx + j) >= src_size) {
                break;
            }

            uint8_t b = src8[src_idx + j];
            switch (j) {
                case 0: {
                    block[0]  = (b & 0xFC) >> 2;
                    block[1]  = (b & 0x03) << 4;
                } break;
                case 1: {
                    block[1] |= (b & 0xF0) >> 4;
                    block[2]  = (b & 0x0F) << 2;
                } break;
                case 2: {
                    block[2] |= (b & 0xC0) >> 6;
                    block[3]  = (b & 0x3F);
                } break;
            }
        }

        for (int j = 0; j < 4; j++) {
            if (block[j] < 26) {
                block[j] += 'A'; // A - Z
            } else if (block[j] >= 26 && block[j] < 52) {
                block[j] += 'a' - 26; // a - z
            } else if (block[j] >= 52 && block[j] < 62) {
                block[j] += '0' - 52; // 0 - 9
            } else if (block[j] == 62) {
                block[j] = '+';
            } else if (block[j] == 63) {
                block[j] = '/';
            } else if (block[j] == 0xFF) {
                block[j] = '=';
            }
            dst8[dst_idx + j] = block[j];
        }

        src_idx += 3;
        dst_idx += 4;
    }

    return encoded_size;
}

size_t hgl_base64_decode(void *dst, const void *src, size_t src_size)
{
    assert(src_size % 4 == 0);
    uint8_t *dst8 = (uint8_t *) dst;
    const uint8_t *src8 = (const uint8_t *) src;
    size_t src_idx = 0;
    size_t dst_idx = 0;
    size_t n_blocks = src_size / 4;
    size_t n_pad_bytes = 0;
    size_t decoded_size_upper_bound = 3 * n_blocks;

    /* dst8 == NULL? Just calculate decoded size and return */
    if (dst8 == NULL) {
        if (src8 == NULL) {
            return decoded_size_upper_bound;
        }
        size_t n = (n_blocks - 1) * 4;
        uint8_t block[4] = {src8[n], src8[n + 1], src8[n + 2], src8[n + 3]};
        n_pad_bytes = 4;
        for (int i = 0; i < 4; i++) {
            if (block[i] == '=') {
                break;
            }
            n_pad_bytes--;
        }
        return decoded_size_upper_bound - n_pad_bytes;
    }

    for (size_t i = 0; i < n_blocks; i++) {
        uint8_t block[4] = {src8[src_idx],
                            src8[src_idx + 1],
                            src8[src_idx + 2],
                            src8[src_idx + 3]};

        int j;
        for (j = 0; j < 4; j++) {
            if (block[j] >= 'A' && block[j] <= 'Z') {
                block[j] -= 'A';
            } else if (block[j] >= 'a' && block[j] <= 'z') {
                block[j] -= 'a' - 26;
            } else if (block[j] >= '0' && block[j] <= '9') {
                block[j] -= '0' - 52;
            } else if (block[j] == '+') {
                block[j] = 62;
            } else if (block[j] == '/') {
                block[j] = 63;
            } else if (block[j] == '=') {
                break;
            } else {
                assert(0);
            }
        }

        if (j == 4) {
            dst8[dst_idx]     = (block[0] << 2) | (block[1] >> 4);
            dst8[dst_idx + 1] = (block[1] << 4) | (block[2] >> 2);
            dst8[dst_idx + 2] = (block[2] << 6) | block[3];
        } else if (j == 3) {
            dst8[dst_idx]     = (block[0] << 2) | (block[1] >> 4);
            dst8[dst_idx + 1] = (block[1] << 4) | (block[2] >> 2);
            n_pad_bytes = 1;
        } else if (j == 2) {
            dst8[dst_idx]     = (block[0] << 2) | (block[1] >> 4);
            n_pad_bytes = 2;
        } else if (j == 1) {
            dst8[dst_idx]     = (block[0] << 2);
            n_pad_bytes = 3;
        }

        dst_idx += 3;
        src_idx += 4;
    }

    return decoded_size_upper_bound - n_pad_bytes;
}

#endif /* HGL_BASE64_IMPLEMENTATION */
