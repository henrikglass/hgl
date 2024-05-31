
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
 * hgl_hamming.h implements a Hamming(16,11) encoding and decoding system.
 *
 *
 * USAGE:
 *
 * Include hgl_hamming.h file like this:
 *
 *     #define HGL_HAMMING_IMPLEMENTATION
 *     #include "hgl_hamming.h"
 *
 * Encode example:
 *
 *     // message to encode
 *     uint8_t data[110] = "Hello world! :)";
 *
 *     // allocate memory for encoded message
 *     size_t encoded_size = HGL_HAMMING_16_11_ENDCODED_SIZE(sizeof(data));
 *     HglHamming16_11 *encoded = malloc(encoded_size);
 *
 *     // encode message
 *     int err = hgl_hamming_encode_16_11(encoded, encoded_size, data, sizeof(data));
 *     if (err != 0) {
 *         printf("Encode error: Invalid argument(s).\n");
 *         return 1;
 *     }
 *
 * Decode example:
 *
 *     // allocate memory for decoded message
 *     size_t decoded_size = HGL_HAMMING_16_11_DECODED_SIZE(encoded_size);
 *     uint8_t *decoded = malloc(decoded_size);
 *
 *     // decode message
 *     int ret = hgl_hamming_decode_16_11(decoded, decoded_size, encoded, encoded_size);
 *     if (ret < 0) {
 *         if (errno == EINVAL) {
 *             printf("Decode error: Invalid argument(s).\n");
 *         } else {
 *             printf("Decode error: Found %d corrupted blocks.\n", -ret);
 *         }
 *         return 1;
 *     }
 *
 *     printf("Decode succeeded. %d/%zu bits needed correcting.\n", ret, 8*decoded_size);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_HAMMING_H
#define HGL_HAMMING_H

#include <stdint.h>
#include <stddef.h>

/**
 * See description for `hgl_hamming_encode_16_11`.
 */
#define HGL_HAMMING_16_11_ENDCODED_SIZE(data_size) \
    ((((data_size) + 10)/11)*8*sizeof(HglHamming16_11))

/**
 * See description for `hgl_hamming_decode_16_11`.
 */
#define HGL_HAMMING_16_11_DECODED_SIZE(encoded_size) \
    (((encoded_size)*11)/(8*sizeof(HglHamming16_11)))

/**
 * Rounds up `size` to the nearest multiple of `factor`.
 */
#define HGL_HAMMING_ROUND_UP(size, factor) \
    (((size) + (factor) - 1) - (((size) + (factor) - 1) % (factor)))

typedef uint16_t HglHamming16_11;

/**
 * Encodes the 11 least significant bits of `data` as a single Hamming(16,11)
 * block:
 *                    _______
 *       bit 15 ---> |B|P|P|i|
 *                   |P|i|i|i|
 *                   |P|i|i|i|
 *                   |i|i|i|i| <--- bit 0
 * or
 *       bit:    15 ...                             ... 2 1 0
 *              | B | P P | i | P | i i i | P | i i i i i i i |
 *
 * Where B is the block parity bit, P are the Hamming(15,11) parity bits, and i
 * are the information bits.
 *
 */
static inline HglHamming16_11 hgl_hamming_encode_16_11_block(uint16_t data);

/**
 * Decodes a single Hamming(16,11) block and returns the (possibly
 * reconstructed) information bits in the 11 least significant bits of the
 * return value. The remaining 5 most significant bits contain meta information
 * obtained from the decoding process. Bit 15, the decode error (DE) bit, if
 * set to 1, indicates that the attempt to decode the block failed; i.e. there
 * was more than one bit error. Bits 11-14, if non-zero, indicates the position
 * of a single bit error in the original block `encoded`.
 *
 *     bit:   15                                 0
 *          | DE | P P P P | i i i i i i i i i i i |
 *
 */
static inline uint16_t hgl_hamming_decode_16_11_block(HglHamming16_11 encoded);

/**
 * Encodes an array of bytes `data` as an array of Hamming(16,11) blocks
 * `encoded`. The size of `data`, i.e. `data_size`, must be a multiple of 11.
 * The size of `encoded`, i.e. `encoded_size`, must be a multiple of 16. In
 * other words, the number of Hamming(16,11) blocks must be a multiple of 8.
 * Furthermore, `encoded_size` must be large enough to fit `data` when
 * encoded. You can use HGL_HAMMING_16_11_ENDCODED_SIZE(`data_size`) to find
 * the smallest value for `encoded_size` which fits the constraints.
 */
int hgl_hamming_encode_16_11(HglHamming16_11 *encoded,
                             size_t encoded_size,
                             uint8_t *data,
                             size_t data_size);

/**
 * Decodes an array of Hamming(16,11) blocks `encoded`, to an array of bytes
 * `data`. The size of `encoded`, i.e. `encoded_size`, must be a multiple of
 * 16. Furthermore, `data_size` must be large enough to fit the `encoded` when
 * decoded. You can use HGL_HAMMING_16_11_DECODED_SIZE(`encoded_size`) to find
 * the smallest value for `data_size` which fits the constraints.
 */
int hgl_hamming_decode_16_11(uint8_t *data,
                             size_t data_size,
                             HglHamming16_11 *encoded,
                             size_t encoded_size);

#endif /* HGL_HAMMING_H */

#ifdef HGL_HAMMING_IMPLEMENTATION

#include <stdio.h>
#include <errno.h>
#include <assert.h>

HglHamming16_11 hgl_hamming_encode_16_11_block(uint16_t data)
{
    assert((data & 0xF800) == 0);
    HglHamming16_11 encoded = 0u;

    /* populate block with payload bits */
    encoded |= data & 0x007F;
    encoded |= (data << 1) & 0x0700;
    encoded |= (data << 2) & 0x1000;

    /* calculate parity bits */
    encoded |= (__builtin_popcount(encoded & 0x00FF) & 1) << 7;  /* parity: bottom 2 rows */
    encoded |= (__builtin_popcount(encoded & 0x0F0F) & 1) << 11; /* parity: odd rows */
    encoded |= (__builtin_popcount(encoded & 0x3333) & 1) << 13; /* parity: rightmost 2 columns */
    encoded |= (__builtin_popcount(encoded & 0x5555) & 1) << 14; /* parity: odd columns */
    encoded |= (__builtin_popcount(encoded) & 1) << 15;          /* parity: entire block */

    return encoded;
}

uint16_t hgl_hamming_decode_16_11_block(HglHamming16_11 encoded)
{
    uint16_t decoded = 0u;

    /* Calculate position of possible single bit error */
    uint16_t error_pos = 0u;
    error_pos |= (__builtin_popcount(encoded & 0x00FF) & 1) << 3;
    error_pos |= (__builtin_popcount(encoded & 0x0F0F) & 1) << 2;
    error_pos |= (__builtin_popcount(encoded & 0x3333) & 1) << 1;
    error_pos |= (__builtin_popcount(encoded & 0x5555) & 1);

    /* Correct the single bit error */
    if (error_pos != 0) {
        encoded ^= (1 << (15 - error_pos));
        decoded |= error_pos << 11;
    }

    /* Extract payload */
    decoded |= (encoded & 0x007F);
    decoded |= (encoded & 0x0700) >> 1;
    decoded |= (encoded & 0x1000) >> 2;

    /* populate meta info bits */
    decoded |= (__builtin_popcount(encoded) & 1) << 15; // decode error

    return decoded;
}


int hgl_hamming_encode_16_11(HglHamming16_11 *encoded,
                         size_t encoded_size,
                         uint8_t *data,
                         size_t data_size)
{
    if (data_size % 11 != 0) {
#ifdef HGL_HAMMING_DEBUG_PRINTS
        fprintf(stderr, "[Error] hgl_hamming_encode_16_11: `data_size` is not a multiple of 11");
#endif
        errno = EINVAL;
        return -1;
    }

    if (11*encoded_size < 16*data_size) {
#ifdef HGL_HAMMING_DEBUG_PRINTS
        fprintf(stderr, "[Error] hgl_hamming_encode_16_11: `encoded_size` is too small.");
#endif
        errno = EINVAL;
        return -1;
    }

    if (((encoded_size / sizeof(HglHamming16_11)) % 8) != 0) {
#ifdef HGL_HAMMING_DEBUG_PRINTS
        fprintf(stderr, "[Error] hgl_hamming_encode_16_11: `encoded_size` is not a multiple of 16.");
#endif
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < data_size / 11; i++) {

        /* split next 11 bytes into 8 11-bit chunks */
        uint16_t raw_uncoded_11[8];
        raw_uncoded_11[0] =  (data[11*i + 0] << 3) |
                             (data[11*i + 1]  & 0xE0 /*0b11100000*/) >>  5;
        raw_uncoded_11[1] = ((data[11*i + 1]  & 0x1F /*0b00011111*/) <<  6) |
                            ((data[11*i + 2]  & 0xFC /*0b11111100*/) >>  2);
        raw_uncoded_11[2] = ((data[11*i + 2]  & 0x03 /*0b00000011*/) <<  9) |
                            ((data[11*i + 3]  & 0xFF /*0b11111111*/) <<  1) |
                            ((data[11*i + 4]  & 0x80 /*0b10000000*/) >>  7);
        raw_uncoded_11[3] = ((data[11*i + 4]  & 0x7F /*0b01111111*/) <<  4) |
                            ((data[11*i + 5]  & 0xF0 /*0b11110000*/) >>  4);
        raw_uncoded_11[4] = ((data[11*i + 5]  & 0x0F /*0b00001111*/) <<  7) |
                            ((data[11*i + 6]  & 0xFE /*0b11111110*/) >>  1);
        raw_uncoded_11[5] = ((data[11*i + 6]  & 0x01 /*0b00000001*/) << 10) |
                            ((data[11*i + 7]  & 0xFF /*0b11111111*/) <<  2) |
                            ((data[11*i + 8]  & 0xC0 /*0b11000000*/) >>  6);
        raw_uncoded_11[6] = ((data[11*i + 8]  & 0x3F /*0b00111111*/) <<  5) |
                            ((data[11*i + 9]  & 0xF8 /*0b11111000*/) >>  3);
        raw_uncoded_11[7] = ((data[11*i + 9]  & 0x07 /*0b00000111*/) <<  8) |
                             (data[11*i + 10] & 0xFF /*0b11111111*/);

        /* encode 8 blocks */
        encoded[i*8 + 0] = hgl_hamming_encode_16_11_block(raw_uncoded_11[0]);
        encoded[i*8 + 1] = hgl_hamming_encode_16_11_block(raw_uncoded_11[1]);
        encoded[i*8 + 2] = hgl_hamming_encode_16_11_block(raw_uncoded_11[2]);
        encoded[i*8 + 3] = hgl_hamming_encode_16_11_block(raw_uncoded_11[3]);
        encoded[i*8 + 4] = hgl_hamming_encode_16_11_block(raw_uncoded_11[4]);
        encoded[i*8 + 5] = hgl_hamming_encode_16_11_block(raw_uncoded_11[5]);
        encoded[i*8 + 6] = hgl_hamming_encode_16_11_block(raw_uncoded_11[6]);
        encoded[i*8 + 7] = hgl_hamming_encode_16_11_block(raw_uncoded_11[7]);
    }

    return 0;
}

int hgl_hamming_decode_16_11(uint8_t *data,
                             size_t data_size,
                             HglHamming16_11 *encoded,
                             size_t encoded_size)
{
    errno = 0;
    size_t n_major_blocks = (encoded_size / sizeof(HglHamming16_11)) / 8;
    int total_corrected_blocks = 0;
    int total_corrupted_blocks = 0;

    if (11*encoded_size > 16*data_size) {
#ifdef HGL_HAMMING_DEBUG_PRINTS
        fprintf(stderr, "[Error] hgl_hamming_decode_16_11: `data_size` is too small.");
#endif
        errno = EINVAL;
        return -1;
    }

    if (((encoded_size / sizeof(HglHamming16_11)) % 8) != 0) {
#ifdef HGL_HAMMING_DEBUG_PRINTS
        fprintf(stderr, "[Error] hgl_hamming_decode_16_11: `encoded_size` is not a multiple of 8.");
#endif
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < n_major_blocks; i++) {
        /* decode 8 Hamming(16,11) blocks (i.e. a major block) consisting of 18 == 88 data bits */
        uint16_t raw_decoded[8];
        for (int j = 0; j < 8; j++) {
            raw_decoded[j] = hgl_hamming_decode_16_11_block(encoded[i*8 + j]);
            if ((raw_decoded[j] & 0x7800) != 0) {
                total_corrected_blocks++;
                total_corrupted_blocks += (raw_decoded[j] >> 15);
            }
        }

        /*
         * Pack raw decoded data into next 11 bytes of `data`.
         */
        data[i*11 + 0]  = ((raw_decoded[0] & 0x07F8 /* 0b111_1111_1000 */) >>  3);
        data[i*11 + 1]  = ((raw_decoded[0] & 0x0007 /* 0b000_0000_0111 */) <<  5) +
                          ((raw_decoded[1] & 0x07C0 /* 0b111_1100_0000 */) >>  6);
        data[i*11 + 2]  = ((raw_decoded[1] & 0x003F /* 0b000_0011_1111 */) <<  2) +
                          ((raw_decoded[2] & 0x0600 /* 0b110_0000_0000 */) >>  9);
        data[i*11 + 3]  = ((raw_decoded[2] & 0x01FE /* 0b001_1111_1110 */) >>  1);
        data[i*11 + 4]  = ((raw_decoded[2] & 0x0001 /* 0b000_0000_0001 */) <<  7) +
                          ((raw_decoded[3] & 0x07F0 /* 0b111_1111_0000 */) >>  4);
        data[i*11 + 5]  = ((raw_decoded[3] & 0x000F /* 0b000_0000_1111 */) <<  4) +
                          ((raw_decoded[4] & 0x0780 /* 0b111_1000_0000 */) >>  7);
        data[i*11 + 6]  = ((raw_decoded[4] & 0x007F /* 0b000_0111_1111 */) <<  1) +
                          ((raw_decoded[5] & 0x0400 /* 0b100_0000_0000 */) >> 10);
        data[i*11 + 7]  = ((raw_decoded[5] & 0x03FC /* 0b011_1111_1100 */) >>  2);
        data[i*11 + 8]  = ((raw_decoded[5] & 0x0003 /* 0b000_0000_0011 */) <<  6) +
                          ((raw_decoded[6] & 0x07E0 /* 0b111_1110_0000 */) >>  5);
        data[i*11 + 9]  = ((raw_decoded[6] & 0x001F /* 0b000_0001_1111 */) <<  3) +
                          ((raw_decoded[7] & 0x0700 /* 0b111_0000_0000 */) >>  8);
        data[i*11 + 10] =  (raw_decoded[7] & 0x00FF /* 0b000_1111_1111 */);
    }

    if (total_corrupted_blocks == 0) {
        return total_corrected_blocks;
    }

    return -total_corrupted_blocks;

}

#endif /* HGL_HAMMING_IMPLEMENTATION */
