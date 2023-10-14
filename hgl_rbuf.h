
/**
 * LICENSE:
 *
 * MIT License
 * 
 * Copyright (c) 2023 Henrik A. Glass
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
 * hgl_ringbuf.h implements a simple to use ring (circular) buffer.
 *
 *
 * USAGE:
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

/*--- vector-specific macros ------------------------------------------------------------*/

/* CONFIGURABLE: HGL_RBUF_TYPE & HGL_RBUF_TYPE_ID */
#ifndef HGL_RBUF_TYPE
#define HGL_RBUF_TYPE void *
#define HGL_RBUF_TYPE_ID voidp
#endif /* HGL_RBUF_TYPE */

/* CONFIGURABLE: HGL_RBUF_ALLOCATOR, HGL_RBUF_REALLOCATOR, HGL_RBUF_FREE */
#if !defined(HGL_RBUF_ALLOCATOR) && \
        !defined(HGL_RBUF_REALLOCATOR) && \
        !defined(HGL_RBUF_FREE)
#define HGL_RBUF_ALLOCATOR (malloc)
#define HGL_RBUF_REALLOCATOR (realloc)
#define HGL_RBUF_FREE (free)
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_RBUF_STRUCT_NAME _CONCAT3(hgl_, HGL_RBUF_TYPE_ID, _rbuf_t)
typedef struct
{
    HGL_RBUF_TYPE *buf;
    size_t length;
    size_t capacity;
    size_t write_idx;
    size_t read_idx;
} HGL_RBUF_STRUCT_NAME;

/*--- Public variables ------------------------------------------------------------------*/

/*--- ring buffer functions -------------------------------------------------------------*/


#define HGL_RBUF_FUNC_INIT_NAME _CONCAT3(hgl_, HGL_RBUF_TYPE_ID, _rbuf_init)
static inline void HGL_RBUF_FUNC_INIT_NAME(HGL_RBUF_STRUCT_NAME *rbuf, size_t capacity)
{
#ifdef HGL_RBUF_POW2_CAPACITY
    assert((capacity & (capacity - 1)) == 0 && "HGL_RBUF_POW2_CAPACITY was defined but " 
                                                "hgl_rbuf capacity was not a power of 2\n")
#endif
    rbuf->capacity = capacity;
    rbuf->buf = HGL_RBUF_ALLOCATOR(rbuf->capacity * sizeof(HGL_RBUF_TYPE));
    rbuf->write_idx = 0;
    rbuf->read_idx = 0;
}

#define HGL_RBUF_FUNC_FREE_NAME _CONCAT3(hgl_, HGL_RBUF_TYPE_ID, _rbuf_free)
static inline void HGL_RBUF_FUNC_FREE_NAME(HGL_RBUF_STRUCT_NAME *rbuf)
{
    HGL_RBUF_FREE(rbuf->buf);
}

#define HGL_RBUF_FUNC_PUSH_BACK_NAME _CONCAT3(hgl_, HGL_RBUF_TYPE_ID, _rbuf_push_back)
static inline int HGL_RBUF_FUNC_PUSH_BACK_NAME(HGL_RBUF_STRUCT_NAME *rbuf, HGL_RBUF_TYPE elem)
{
#ifdef HGL_RBUF_POW2_CAPACITY
    if (((rbuf->write_idx + 1) & (rbuf->capacity - 1)) == rbuf->read_idx) {
#else
    if (((rbuf->write_idx + 1) % rbuf->capacity) == rbuf->read_idx) {
#endif
        return -1; // buffer full
    }
   
    rbuf->buf[rbuf->write_idx] = elem;
    rbuf->write_idx++;
#ifdef HGL_RBUF_POW2_CAPACITY
    rbuf->write_idx &= rbuf->capacity - 1;
#else
    rbuf->write_idx %= rbuf->capacity;
#endif
    rbuf->length++;
    return 0;
}

#define HGL_RBUF_FUNC_POP_FRONT_NAME _CONCAT3(hgl_, HGL_RBUF_TYPE_ID, _rbuf_pop_front)
static inline HGL_RBUF_TYPE *HGL_RBUF_FUNC_POP_FRONT_NAME(HGL_RBUF_STRUCT_NAME *rbuf)
{
    if (rbuf->write_idx == rbuf->read_idx) {
        return NULL; // buffer empty
    }

    HGL_RBUF_TYPE *retval = &rbuf->buf[rbuf->read_idx];
    rbuf->read_idx++;
#ifdef HGL_RBUF_POW2_CAPACITY
    rbuf->read_idx &= rbuf->capacity - 1;
#else
    rbuf->read_idx %= rbuf->capacity;
#endif
    rbuf->length--;

    return retval;
   
}
