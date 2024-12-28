
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
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
 * hgl_rle.h implements run-length encoding and decoding with multiple possible
 * run-length and data block sizes.
 *
 *
 * USAGE:
 *
 * Include hgl_rle.h file like this:
 *
 *     #define HGL_RLE_IMPLEMENTATION
 *     #include "hgl_rle.h"
 *
 * HGL_RLE_IMPLEMENTATION must only be defined once, in a single compilation unit.
 *
 *
 * EXAMPLE:
 *
 *     ssize_t size;
 *     uint32_t *data = obtain_some_data(&size);
 *     uint8_t *encoded_data;
 *
 *     // figure out the encoded size of `data`
 *     ssize_t encoded_size = hgl_rle_encode32_8(NULL, data, size);
 *         
 *     // allocate memory for encoded data
 *     encoded_data = malloc(encoded_size);
 *
 *     // encode data and place into `encoded_data`.
 *     hgl_rle_encode32_8(encoded_data, data, size);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_RLE_H
#define HGL_RLE_H

#include <sys/types.h>
#include <stdint.h>

/**
 * Run-length encodes `size` bytes of data at `src` and places it into `dst`. The 
 * encoding is performed on byte-sized (8 bit) elements. Returns the size in bytes 
 * of the run-length encoded data. If `dst` is NULL, then only the encoded size is 
 * calculcated and returned.
 *
 * Each run-length encoded segment of data is stored as a pair of two 8 bit values,
 * a run-length `RL` and the data `DA`. The run-length of the segment is stored on the
 * lower address. The data is stored on the higher address. Note that a run-length 
 * value of N indicates an actual run length of N+1. I.e. if `RL`=5, then `DA` should
 * be repeated 6 times.
 *
 * RL/DA pair layout:
 *
 *                        |_RL_|_DA_|
 *     address offset:    0    1    2
 *
 */
ssize_t hgl_rle_encode8_8(uint8_t *restrict dst, const uint8_t *restrict src, size_t size);

/**
 * Decodes `size` bytes of run-length encoded data at `src` and places it into `dst`.
 * Returns the size in bytes of the decoded data. If `dst` is NULL, then only the decoded 
 * size is calculcated and returned.
 *
 * Note: this function assumes that `src` contains encoded data formatted in the way 
 * described in `hgl_rle_encode8_8`.
*/
ssize_t hgl_rle_decode8_8(uint8_t *restrict dst, const uint8_t *restrict src, size_t size);

/**
 * Run-length encodes `size` bytes of data at `src` and places it into `dst`. The 
 * encoding is performed on 32 bit-sized elements. Returns the size in bytes of the 
 * run-length encoded data. If `dst` is NULL, then only the encoded size is 
 * calculcated and returned.
 *
 * Each run-length encoded segment of data is stored as a pair of two 32 bit values,
 * a run-length `RL` and the data `DA`. The run-length of the segment is stored on the
 * lower address. The data is stored on the higher address. Note that a run-length 
 * value of N indicates an actual run length of N+1. I.e. if `RL`=5, then `DA` should
 * be repeated 6 times.
 *
 * Note: Both `RL` and `DA` are stored in the byte-order native to the machine running
 * the program. Hence, if data is to be decoded on a different machine, care should be 
 * taken by the programmer to ensure correct byte ordering. 
 *
 * RL/DA pair layout:
 *
 *                        |_____RL_____|______DA_____|
 *     address offset:    0            4             8
 *
 */
ssize_t hgl_rle_encode32_32(uint32_t *restrict dst, const uint32_t *restrict src, size_t size);

 /**
 * Decodes `size` bytes of run-length encoded data at `src` and places it into `dst`.
 * Returns the size in bytes of the decoded data. If `dst` is NULL, then only the decoded 
 * size is calculcated and returned.
 *
 * Note: this function assumes that `src` contains encoded data formatted in the way 
 * described in `hgl_rle_encode32_32`.
 */
ssize_t hgl_rle_decode32_32(uint32_t *restrict dst, const uint32_t *restrict src, size_t size);

/**
 * Run-length encodes `size` bytes of data at `src` and places it into `dst`. The 
 * encoding is performed on 32 bit-sized elements. Returns the size in bytes of the 
 * run-length encoded data. If `dst` is NULL, then only the encoded size is 
 * calculcated and returned.
 *
 * Each run-length encoded segment of data is stored as a pair of two values, consisting
 * of an 8-bit run-length value `RL` and a 32-bit data value `DA`. The run-length of 
 * the segment is stored on the lower address. The data is stored on the higher address. 
 * Note that a run-length value of N indicates an actual run length of N+1. I.e. if 
 * `RL`=5, then `DA` should be repeated 6 times.
 *
 * Note: `DA` is stored in big endian byte-order.
 *
 * RL/DA pair layout:
 *
 *                        |_RL_|______DA_____|
 *     address offset:    0    1             5
 *
 */
ssize_t hgl_rle_encode32_8(uint8_t *restrict dst, const uint32_t *restrict src, size_t size);

 /**
 * Decodes `size` bytes of run-length encoded data at `src` and places it into `dst`.
 * Returns the size in bytes of the decoded data. If `dst` is NULL, then only the decoded 
 * size is calculcated and returned.
 *
 * Note: this function assumes that `src` contains encoded data formatted in the way 
 * described in `hgl_rle_encode32_8`.
 */
ssize_t hgl_rle_decode32_8(uint32_t *restrict dst, const uint8_t *restrict src, size_t size);

#endif /* HGL_RLE_H */

#ifdef HGL_RLE_IMPLEMENTATION

#include <stdio.h> // DEBUG
#include <assert.h> // DEBUG
#include <string.h>

#define MEMSET32(dst, val, n)              \
    do {                                   \
        for (size_t i = 0; i < (n); i++) { \
            (dst)[i] = (val);              \
        }                                  \
    } while (0)

static inline void memset32_(void *dst, uint32_t val, size_t n)
{
    //uint32_t *dst32 = (uint32_t *) dst;
    //uint32_t c = (uint32_t) n;
    //(void) val;
    //for (size_t i = 0; i < n; i++) {
    //    dst32[i] = val;
    //}
    __asm__ __volatile__ (
        "mov %1, %%eax\n\t"
        "movq %0, %%rcx\n\t"
        "movq %2, %%rdi\n\t"
        "cld\n\t"
        "rep\n\t"
        "stosl"
        :
        : "r" (n), "r" (val), "r" (dst)
        : "ecx", "eax", "rdi"
    );
}

ssize_t hgl_rle_encode8_8(uint8_t *restrict dst, const uint8_t *restrict src, size_t size)
{
    size_t write_idx = 0;
    uint8_t rl = 1;
    uint8_t da = src[0];

    for (size_t i = 1; i < size; i++) {
        uint8_t next = src[i];

        if ((rl == 0) || (next != da)) {
            if (dst != NULL) {
                dst[write_idx]     = rl - 1;
                dst[write_idx + 1] = da;
            }
            write_idx += 2;
            da = next;
            rl = 1;
        } else {
            rl++;
        }
    }

    if (dst != NULL) {
        dst[write_idx]     = rl - 1;
        dst[write_idx + 1] = da;
    }
    write_idx += 2;

    assert(write_idx % 2 == 0);
    return write_idx;
}

ssize_t hgl_rle_decode8_8(uint8_t *restrict dst, const uint8_t *restrict src, size_t size)
{ 
    if((size & 1) != 0) {
        fprintf(stderr, "[hgl_rle_decode8_8] Error: `size` must be divisible by 2");
        return -1;
    }

    size_t write_idx = 0;
    for (size_t i = 0; i < size; i += 2) {
        size_t rl = ((size_t)src[i]) + 1;
        uint8_t da = src[i + 1];
        if (dst != NULL) {
            memset(&dst[write_idx], da, rl);
        }
        write_idx += rl;
    }

    return write_idx;
} 

ssize_t hgl_rle_encode32_32(uint32_t *restrict dst, const uint32_t *restrict src, size_t size)
{
    size_t write_idx = 0;
    uint32_t rl = 1;
    uint32_t da = src[0];

    if((size & 3) != 0) {
        fprintf(stderr, "[hgl_rle_encode32_32] Error: `size` must be divisible by 4");
        return -1;
    }

    for (size_t i = 1; i < size / sizeof(uint32_t); i++) {
        uint32_t next = src[i];

        if ((rl == 0) || (next != da)) {
            if (dst != NULL) {
                dst[write_idx]     = rl - 1;
                dst[write_idx + 1] = da;
            }
            write_idx += 2;
            da = next;
            rl = 1;
        } else {
            rl++;
        }
    }

    if (dst != NULL) {
        dst[write_idx]     = rl - 1;
        dst[write_idx + 1] = da;
    }
    write_idx += 2;

    assert(write_idx % 2 == 0);
    return write_idx * sizeof(uint32_t);
}

ssize_t hgl_rle_decode32_32(uint32_t *restrict dst, const uint32_t *restrict src, size_t size)
{
    if((size & 7) != 0) {
        fprintf(stderr, "[hgl_rle_decode32_32] Error: `size` must be divisible by 8");
        return -1;
    }

    size_t write_idx = 0;
    for (size_t i = 0; i < size / sizeof(uint32_t); i += 2) {
        size_t rl = ((size_t)src[i]) + 1;
        uint32_t da = src[i + 1];
        if (dst != NULL) {
            memset32_(&dst[write_idx], da, rl);
        }
        write_idx += rl;
    }

    return write_idx * sizeof(uint32_t);
}

ssize_t hgl_rle_encode32_8(uint8_t *restrict dst, const uint32_t *restrict src, size_t size)
{
    size_t write_idx = 0;
    uint8_t rl = 1;
    uint32_t da = src[0];

    if((size & 3) != 0) {
        fprintf(stderr, "[hgl_rle_encode32_8] Error: `size` must be divisible by 4");
        return -1;
    }

    for (size_t i = 1; i < size / sizeof(uint32_t); i++) {
        uint32_t next = src[i];

        if ((rl == 0) || (next != da)) {
            if (dst != NULL) {
                dst[write_idx]     = rl - 1;
                dst[write_idx + 1] = (da >> 24) & 0xFF;
                dst[write_idx + 2] = (da >> 16) & 0xFF;
                dst[write_idx + 3] = (da >>  8) & 0xFF;
                dst[write_idx + 4] = (da >>  0) & 0xFF;
            }
            write_idx += 5;
            da = next;
            rl = 1;
        } else {
            rl++;
        }
    }

    if (dst != NULL) {
        dst[write_idx]     = rl - 1;
        dst[write_idx + 1] = (da >> 24) & 0xFF;
        dst[write_idx + 2] = (da >> 16) & 0xFF;
        dst[write_idx + 3] = (da >>  8) & 0xFF;
        dst[write_idx + 4] = (da >>  0) & 0xFF;
    }
    write_idx += 5;

    assert(write_idx % 5 == 0);
    return write_idx;
}

ssize_t hgl_rle_decode32_8(uint32_t *restrict dst, const uint8_t *restrict src, size_t size)
{
    if((size % 5) != 0) {
        fprintf(stderr, "[hgl_rle_decode32_8] Error: `size` must be divisible by 5");
        return -1;
    }

    size_t write_idx = 0;
    for (size_t i = 0; i < size; i += 5) {
        size_t rl = ((size_t)src[i]) + 1;
        uint32_t da = 0;
        da |= src[i + 1] << 24;
        da |= src[i + 2] << 16;
        da |= src[i + 3] <<  8;
        da |= src[i + 4] <<  0;
        if (dst != NULL) {
            memset32_(&dst[write_idx], da, rl);
        }
        write_idx += rl;
    }

    return write_idx * sizeof(uint32_t);
}

#endif

