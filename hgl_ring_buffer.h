
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
 * Include hgl_ring_buffer.h file like this:
 *
 *     #define HGL_RING_BUFFER_TYPE char *
 *     #define HGL_RING_BUFFER_TYPE_ID charp
 *     #include "hgl_ring_buffer.h"
 *
 * This will create an implementation of hgl_ring_buffer capable of holding an element of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_ring_buffer_init(hgl_charp_ring_buffer_t *rbuf, size_t capacity);
 *     void hgl_charp_ring_buffer_free(hgl_charp_ring_buffer_t *rbuf);
 *     void hgl_charp_ring_buffer_push_back(hgl_charp_ring_buffer_t *rbuf, char **item);
 *     void hgl_charp_ring_buffer_push_back_value(hgl_charp_ring_buffer_t *rbuf, char *item);
 *     char *hgl_charp_ring_buffer_pop_front(hgl_charp_ring_buffer_t *rbuf);
 *
 * HGL_RING_BUFFER_TYPE and HGL_RING_BUFFER_TYPE_ID may be redefined and hgl_ring_buffer.h included multiple times
 * to create implementations of hgl_ring_buffer for different types:
 *
 *     #define HGL_RING_BUFFER_TYPE char *
 *     #define HGL_RING_BUFFER_TYPE_ID charp
 *     #include "hgl_ring_buffer.h"
 *
 *     #undef HGL_RING_BUFFER_TYPE
 *     #undef HGL_RING_BUFFER_TYPE_ID
 *     #define HGL_RING_BUFFER_TYPE int
 *     #define HGL_RING_BUFFER_TYPE_ID int
 *     #include "hgl_ring_buffer.h"
 *
 * If HGL_RING_BUFFER_TYPE and HGL_RING_BUFFER_TYPE_ID are left undefined, the default element type of
 * hgl_ring_buffer will be void *, and the default element type identifier will be "voidp".
 *
 * hgl_ring_buffer allows the default allocator, reallocator and free function to be overridden by
 * redefining the following defines before including hgl_ring_buffer.h:
 *
 *     #define HGL_RING_BUFFER_ALLOCATOR                (malloc)
 *     #define HGL_RING_BUFFER_REALLOCATOR              (realloc)
 *     #define HGL_RING_BUFFER_FREE                     (free)
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

/*--- vector-specific macros ------------------------------------------------------------*/

/* CONFIGURABLE: HGL_RING_BUFFER_TYPE & HGL_RING_BUFFER_TYPE_ID */
#ifndef HGL_RING_BUFFER_TYPE
#define HGL_RING_BUFFER_TYPE void *
#define HGL_RING_BUFFER_TYPE_ID voidp
#endif /* HGL_RING_BUFFER_TYPE */

/* CONFIGURABLE: HGL_RING_BUFFER_ALLOCATOR, HGL_RING_BUFFER_REALLOCATOR, HGL_RING_BUFFER_FREE */
#if !defined(HGL_RING_BUFFER_ALLOCATOR) && \
        !defined(HGL_RING_BUFFER_REALLOCATOR) && \
        !defined(HGL_RING_BUFFER_FREE)
#define HGL_RING_BUFFER_ALLOCATOR (malloc)
#define HGL_RING_BUFFER_REALLOCATOR (realloc)
#define HGL_RING_BUFFER_FREE (free)
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_RING_BUFFER_STRUCT _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_t)
typedef struct
{
    HGL_RING_BUFFER_TYPE *buf;
    size_t length;
    size_t capacity;
    size_t write_idx;
    size_t read_idx;
} HGL_RING_BUFFER_STRUCT;

/*--- Public variables ------------------------------------------------------------------*/

/*--- ring buffer functions -------------------------------------------------------------*/


#define HGL_RING_BUFFER_FUNC_INIT _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_init)
static inline void HGL_RING_BUFFER_FUNC_INIT(HGL_RING_BUFFER_STRUCT *rbuf, size_t capacity)
{
#ifdef HGL_RING_BUFFER_POW2_CAPACITY
    assert((capacity & (capacity - 1)) == 0 && "HGL_RING_BUFFER_POW2_CAPACITY was defined but "
                                                "hgl_ring_buffer capacity was not a power of 2\n");
#endif
    rbuf->capacity = capacity;
    rbuf->buf = HGL_RING_BUFFER_ALLOCATOR(rbuf->capacity * sizeof(HGL_RING_BUFFER_TYPE));
    rbuf->write_idx = 0;
    rbuf->read_idx = 0;
}

#define HGL_RING_BUFFER_FUNC_FREE _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_free)
static inline void HGL_RING_BUFFER_FUNC_FREE(HGL_RING_BUFFER_STRUCT *rbuf)
{
    HGL_RING_BUFFER_FREE(rbuf->buf);
}

#define HGL_RING_BUFFER_FUNC_PUSH_BACK _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_push_back)
static inline int HGL_RING_BUFFER_FUNC_PUSH_BACK(HGL_RING_BUFFER_STRUCT *rbuf, 
                                                 HGL_RING_BUFFER_TYPE *elem)
{
#ifdef HGL_RING_BUFFER_POW2_CAPACITY
    if (((rbuf->write_idx + 1) & (rbuf->capacity - 1)) == rbuf->read_idx) {
#else
    if (((rbuf->write_idx + 1) % rbuf->capacity) == rbuf->read_idx) {
#endif
        return -1; // buffer full
    }

    rbuf->buf[rbuf->write_idx] = *elem;
    rbuf->write_idx++;
#ifdef HGL_RING_BUFFER_POW2_CAPACITY
    rbuf->write_idx &= rbuf->capacity - 1;
#else
    rbuf->write_idx %= rbuf->capacity;
#endif
    rbuf->length++;
    return 0;
}

#define HGL_RING_BUFFER_FUNC_PUSH_BACK_VALUE _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_push_back_value)
static inline int HGL_RING_BUFFER_FUNC_PUSH_BACK_VALUE(HGL_RING_BUFFER_STRUCT *rbuf, 
                                                       HGL_RING_BUFFER_TYPE elem)
{
    return HGL_RING_BUFFER_FUNC_PUSH_BACK(rbuf, &elem);
}

#define HGL_RING_BUFFER_FUNC_POP_FRONT _CONCAT3(hgl_, HGL_RING_BUFFER_TYPE_ID, _ring_buffer_pop_front)
static inline HGL_RING_BUFFER_TYPE *HGL_RING_BUFFER_FUNC_POP_FRONT(HGL_RING_BUFFER_STRUCT *rbuf)
{
    if (rbuf->write_idx == rbuf->read_idx) {
        return NULL; // buffer empty
    }

    HGL_RING_BUFFER_TYPE *retval = &rbuf->buf[rbuf->read_idx];
    rbuf->read_idx++;
#ifdef HGL_RING_BUFFER_POW2_CAPACITY
    rbuf->read_idx &= rbuf->capacity - 1;
#else
    rbuf->read_idx %= rbuf->capacity;
#endif
    rbuf->length--;

    return retval;

}
