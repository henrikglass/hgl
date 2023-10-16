
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
 * hgl_bufchan.h implements a thread-safe buffered message queue similar to buffered go channels.
 *
 *
 * USAGE:
 *
 * Include hgl_bufchan.h file like this:
 *
 *     #define HGL_BUFCHAN_TYPE char *
 *     #define HGL_BUFCHAN_TYPE_ID charp
 *     #include "hgl_bufchan.h"
 *
 * This will create an implementation of hgl_bufchan capable of holding an element of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_bufchan_init(hgl_charp_bufchan_t *chan, size_t capacity);
 *     void hgl_charp_bufchan_free(hgl_charp_bufchan_t *chan);
 *     void hgl_charp_bufchan_send(hgl_charp_bufchan_t *chan, char **item);
 *     void hgl_charp_bufchan_send_value(hgl_charp_bufchan_t *chan, char *item);
 *     char *hgl_charp_bufchan_recv(hgl_charp_bufchan_t *chan);
 *
 * HGL_BUFCHAN_TYPE and HGL_BUFCHAN_TYPE_ID may be redefined and hgl_bufchan.h included multiple times
 * to create implementations of hgl_bufchan for different types:
 *
 *     #define HGL_BUFCHAN_TYPE char *
 *     #define HGL_BUFCHAN_TYPE_ID charp
 *     #include "hgl_bufchan.h"
 *
 *     #undef HGL_BUFCHAN_TYPE
 *     #undef HGL_BUFCHAN_TYPE_ID
 *     #define HGL_BUFCHAN_TYPE int
 *     #define HGL_BUFCHAN_TYPE_ID int
 *     #include "hgl_bufchan.h"
 *
 * If HGL_BUFCHAN_TYPE and HGL_BUFCHAN_TYPE_ID are left undefined, the default element type of
 * hgl_bufchan will be void *, and the default element type identifier will be "voidp".
 *
 * hgl_bufchan allows the default allocator and corresponding free function to be overridden by
 * redefining the following defines before including hgl_bufchan.h:
 *
 *     #define HGL_BUFCHAN_ALLOCATOR                (malloc)
 *     #define HGL_BUFCHAN_FREE                     (free)
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

#ifdef NDEBUG
#define HGL_ASSERT(expr, msg) do {                       \
    if (!(expr)) {                                       \
        fprintf(stderr, "HGL_ASSERT fail: %s\n", (msg)); \
    }                                                    \
} while(0)
#else
#define HGL_ASSERT(expr, msg) assert((expr) && (msg))
#endif

/*--- buffered channel-specific macros --------------------------------------------------*/

/* CONFIGURABLE: HGL_BUFCHAN_TYPE & HGL_BUFCHAN_TYPE_ID */
#ifndef HGL_BUFCHAN_TYPE
#define HGL_BUFCHAN_TYPE void *
#define HGL_BUFCHAN_TYPE_ID voidp
#endif /* HGL_BUFCHAN_TYPE */

/* CONFIGURABLE: HGL_BUFCHAN_ALLOCATOR, HGL_BUFCHAN_REALLOCATOR, HGL_BUFCHAN_FREE */
#if !defined(HGL_BUFCHAN_ALLOCATOR) && !defined(HGL_BUFCHAN_FREE)
#define HGL_BUFCHAN_ALLOCATOR (malloc)
#define HGL_BUFCHAN_FREE (free)
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_BUFCHAN_STRUCT_NAME _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_t)
typedef struct
{
    HGL_BUFCHAN_TYPE *buf;
    uint32_t read_offset;
    uint32_t write_offset;
    uint32_t capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_writable;
    pthread_cond_t cvar_readable;
} HGL_BUFCHAN_STRUCT_NAME;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Channel functions -----------------------------------------------------------------*/

#define HGL_BUFCHAN_FUNC_INIT_NAME _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_init)
static inline void HGL_BUFCHAN_FUNC_INIT_NAME(HGL_BUFCHAN_STRUCT_NAME *chan, size_t capacity)
{
    HGL_ASSERT((capacity & (capacity -1)) == 0, "bufchan capacity must be a power of 2");
    chan->buf = HGL_BUFCHAN_ALLOCATOR(sizeof(HGL_BUFCHAN_TYPE) * capacity);
    chan->read_offset = 0;
    chan->write_offset = 0;
    chan->capacity = capacity;
    pthread_mutex_init(&chan->mutex, NULL);
    pthread_cond_init(&chan->cvar_writable, NULL);
    pthread_cond_init(&chan->cvar_readable, NULL);
}

#define HGL_BUFCHAN_FUNC_FREE_NAME _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_free)
static inline void HGL_BUFCHAN_FUNC_FREE_NAME(HGL_BUFCHAN_STRUCT_NAME *chan)
{
    pthread_mutex_destroy(&chan->mutex);
    pthread_cond_destroy(&chan->cvar_writable);
    pthread_cond_destroy(&chan->cvar_readable);
    HGL_BUFCHAN_FREE(chan->buf);
}

#define HGL_BUFCHAN_FUNC_SEND_NAME _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_send)
static inline void HGL_BUFCHAN_FUNC_SEND_NAME(HGL_BUFCHAN_STRUCT_NAME *chan, HGL_BUFCHAN_TYPE *item)
{
    pthread_mutex_lock(&chan->mutex);
    /* while not writable... */
    while(((chan->write_offset + 1) & (chan->capacity - 1)) == chan->read_offset) {
        pthread_cond_wait(&chan->cvar_writable, &chan->mutex);
    }
    chan->buf[chan->write_offset] = *item;
    chan->write_offset = (chan->write_offset + 1) & (chan->capacity - 1);
    pthread_cond_signal(&chan->cvar_readable);
    pthread_mutex_unlock(&chan->mutex);
}

#define HGL_BUFCHAN_FUNC_SEND_VALUE _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_send_value)
static inline void HGL_BUFCHAN_FUNC_SEND_VALUE(HGL_BUFCHAN_STRUCT_NAME *chan, HGL_BUFCHAN_TYPE item)
{
    HGL_BUFCHAN_FUNC_SEND_NAME(chan, &item);
}

#define HGL_BUFCHAN_FUNC_RECV_NAME _CONCAT3(hgl_, HGL_BUFCHAN_TYPE_ID, _bufchan_recv)
static inline HGL_BUFCHAN_TYPE HGL_BUFCHAN_FUNC_RECV_NAME(HGL_BUFCHAN_STRUCT_NAME *chan)
{
    pthread_mutex_lock(&chan->mutex);
    /* while not readable... */
    while(chan->write_offset == chan->read_offset) {
        pthread_cond_wait(&chan->cvar_readable, &chan->mutex);
    }
    HGL_BUFCHAN_TYPE item = chan->buf[chan->read_offset];
    chan->read_offset = (chan->read_offset + 1) & (chan->capacity - 1);
    pthread_cond_signal(&chan->cvar_writable);
    pthread_mutex_unlock(&chan->mutex);
    return item;
}
