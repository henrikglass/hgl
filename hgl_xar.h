
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
 * hgl_xar.h is a quick and dirty generic implementation of Andrew Reece's exponential
 * array (A.k.a. "Xar"). The idea, and parts of the implementation details, are taken
 * from his talk "Assuming as Much as Possible" from BSC 2025:
 * https://www.youtube.com/watch?v=i-h95QIGchY
 *
 *
 * USAGE:
 *
 * Just create a struct of the correct form by using the HglXar(T) macro and initialize
 * it with the hgl_xar_make() macro:
 *
 *     typedef HglXar(float) Floats;
 *     Floats fs = hgl_xar_make(float, .shift = MSB32(4096));
 *     hgl_xar_push(&fs, 123.456f);
 *
 *     or:
 *
 *     HglXar(float) fs = hgl_xar_make(float, .alloc = my_alloc, .free = my_free);
 *     hgl_xar_push(&fs, 123.456f);
 *
 *
 * EXAMPLES:
 *
 * See the examples/ directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_XAR_H
#define HGL_XAR_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

/*--- Public macros ---------------------------------------------------------------------*/

#ifndef MSB64
#define MSB64(x) (63 - __builtin_clzll(x))
#endif

#ifndef MSB32
#define MSB32(x) (31 - __builtin_clz(x))
#endif

#define HGL_XAR_N_CHUNKS 30

#define HglXar(T) struct { HglXarHeader header; T *chunks[HGL_XAR_N_CHUNKS]; }

#define hgl_xar_make(T, ...)                                  \
    {                                                         \
        .header = {                                           \
            .shift = MSB32(256),                              \
            .n_chunks = HGL_XAR_N_CHUNKS,                     \
            .el_size = sizeof(T),                             \
            .alloc = malloc,                                  \
            .free = free,                                     \
            .count = 0,                                       \
            __VA_ARGS__                                       \
        },                                                    \
    }
#define hgl_xar_destroy(xar)                                  \
    do {                                                      \
        for (unsigned int i = 0; i < HGL_XAR_N_CHUNKS; i++) { \
            if ((xar)->chunks[i] == NULL) break;              \
            (xar)->header.free((xar)->chunks[i]);             \
            (xar)->chunks[i] = NULL;                          \
        }                                                     \
        (xar)->header.count = 0;                              \
    } while (0)
#define hgl_xar_push(xar, elem)                               \
    do {                                                      \
        __typeof__ (**(xar)->chunks) elem__ = (elem);         \
        hgl_xar_push_generic(&(xar)->header, &elem__);        \
    } while (0)
#define hgl_xar_clear(xar)                                  do { (xar)->header.count = 0; } while (0)
#define hgl_xar_last(xar)                                   hgl_xar_last_generic(&(xar)->header)
#define hgl_xar_pop(xar)                                    hgl_xar_pop_generic(&(xar)->header)
#define hgl_xar_get(xar, i)                                 hgl_xar_get_generic(&(xar)->header, (i))
#define hgl_xar_remove_backswap(xar, i)                     hgl_xar_remove_backswap_generic(&(xar)->header, (i))
#define hgl_xar_copy_first_n_to_array(xar, buf, n)          hgl_xar_copy_to_array_generic(&(xar)->header, (buf), (n))
#define hgl_xar_copy_to_array(xar, buf)                     hgl_xar_copy_to_array_generic(&(xar)->header, (buf), (xar)->header.count)
#define hgl_xar_count(xar)                                  ((xar)->header.count)
#define hgl_xar_to_array(xar, optional_allocator)           hgl_xar_to_array_generic(&(xar)->header, optional_allocator)

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    uint8_t shift;
    uint8_t n_chunks;
    uint32_t el_size;
    void *(*alloc)(size_t);
    void (*free)(void *);
    size_t count;
} HglXarHeader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void hgl_xar_push_generic(HglXarHeader *xar, void *el);
void *hgl_xar_pop_generic(HglXarHeader *xar);
void *hgl_xar_last_generic(HglXarHeader *xar);
void *hgl_xar_get_generic(HglXarHeader *xar, size_t i);
void hgl_xar_remove_backswap_generic(HglXarHeader *xar, size_t i);
size_t hgl_xar_copy_to_array_generic(HglXarHeader *xar, void *buf, size_t n);
void *hgl_xar_to_array_generic(HglXarHeader *xar, void *(*alloc)(size_t));

#endif /* HGL_XAR_H */

#ifdef HGL_XAR_IMPLEMENTATION

#include <string.h>

#define countof(x) (sizeof((x)) / sizeof((x)[0]))

void hgl_xar_push_generic(HglXarHeader *xar, void *el)
{
    uint8_t **chunks = (uint8_t**)(xar + 1);

    size_t  chunk_i   = xar->count;
    size_t  chunk_cap = 1 << xar->shift;
    uint8_t chunks_i  = 0;

    size_t i_shift = xar->count >> xar->shift;
    if (i_shift > 0) {
        chunks_i   = MSB64(i_shift);
        chunk_cap  = 1 << (chunks_i + xar->shift);
        chunk_i   -= chunk_cap;
        chunks_i++;
    }

    if (chunks[chunks_i] == NULL) {
        chunks[chunks_i] = xar->alloc(chunk_cap * xar->el_size);
    }

    memcpy(chunks[chunks_i] + chunk_i * xar->el_size, el, xar->el_size);
    xar->count++;
}

void *hgl_xar_pop_generic(HglXarHeader *xar)
{
    if (xar->count == 0) {
        return NULL;
    }
    void *ret = hgl_xar_get_generic(xar, xar->count - 1);
    xar->count--;
    return ret;
}

void *hgl_xar_last_generic(HglXarHeader *xar)
{
    if (xar->count == 0) {
        return NULL;
    }
    return hgl_xar_get_generic(xar, xar->count - 1);
}

void *hgl_xar_get_generic(HglXarHeader *xar, size_t i)
{
    uint8_t **chunks = (uint8_t**)(xar + 1);

    size_t  chunk_i   = i;
    size_t  chunk_cap = 1 << xar->shift;
    uint8_t chunks_i  = 0;

    size_t i_shift = i >> xar->shift;
    if (i_shift > 0) {
        chunks_i   = MSB64(i_shift);
        chunk_cap  = 1 << (chunks_i + xar->shift);
        chunk_i   -= chunk_cap;
        chunks_i++;
    }

    return chunks[chunks_i] + chunk_i * xar->el_size;
}

void hgl_xar_remove_backswap_generic(HglXarHeader *xar, size_t i)
{
    void *elem = hgl_xar_get_generic(xar, i);
    void *last = hgl_xar_pop_generic(xar);
    memcpy(elem, last, xar->el_size);
}

size_t hgl_xar_copy_to_array_generic(HglXarHeader *xar, void *buf, size_t n)
{
    if (buf == NULL) {
        return xar->el_size * n;
    }

    uint8_t **chunks = (uint8_t**)(xar + 1);
    (void) chunks;
    (void) buf;
    uint8_t *buf8 = (uint8_t *)buf;
    size_t buf_offset = 0;
    size_t copy_size = 0;

    size_t  chunk_i   = n;
    size_t  chunk_cap = 1 << xar->shift;
    uint8_t chunks_i  = 0;

    size_t i_shift = n >> xar->shift;
    if (i_shift > 0) {
        chunks_i   = MSB64(i_shift);
        chunk_cap  = 1 << (chunks_i + xar->shift);
        chunk_i   -= chunk_cap;
        chunks_i++;

        copy_size = (1 << xar->shift) * xar->el_size;
        memcpy(&buf8[0], chunks[0], copy_size);
        buf_offset += copy_size;

        for (uint8_t i = 1; i < chunks_i; i++) {
            copy_size = (1 << (i - 1 + xar->shift)) * xar->el_size;
            memcpy(&buf8[buf_offset], chunks[i], copy_size);
            buf_offset += copy_size;
        }
    }

    copy_size = chunk_i * xar->el_size;
    memcpy(&buf8[buf_offset], chunks[chunks_i], copy_size);
    buf_offset += copy_size;

    return buf_offset;
}

void *hgl_xar_to_array_generic(HglXarHeader *xar, void *(*optional_allocator)(size_t))
{
    size_t size = xar->el_size * xar->count;
    void *buf;
    if (optional_allocator != NULL) {
        buf = optional_allocator(size);
    } else {
        buf = xar->alloc(size);
    }
    hgl_xar_copy_to_array_generic(xar, buf, xar->count);
    return buf;
}

#endif

