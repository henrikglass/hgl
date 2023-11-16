
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
 * hgl_buffered_chan.h implements a thread-safe buffered message queue similar to buffered go channels.
 *
 *
 * USAGE:
 *
 * Include hgl_buffered_chan.h file like this:
 *
 *     #define HGL_BUFFERED_CHAN_TYPE char *
 *     #define HGL_BUFFERED_CHAN_TYPE_ID charp
 *     #include "hgl_buffered_chan.h"
 *
 * This will create an implementation of hgl_buffered_chan capable of holding an element of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_buffered_chan_init(hgl_charp_buffered_chan_t *chan, size_t capacity);
 *     void hgl_charp_buffered_chan_free(hgl_charp_buffered_chan_t *chan);
 *     void hgl_charp_buffered_chan_send(hgl_charp_buffered_chan_t *chan, char **item);
 *     void hgl_charp_buffered_chan_send_value(hgl_charp_buffered_chan_t *chan, char *item);
 *     char *hgl_charp_buffered_chan_recv(hgl_charp_buffered_chan_t *chan);
 *
 * HGL_BUFFERED_CHAN_TYPE and HGL_BUFFERED_CHAN_TYPE_ID may be redefined and hgl_buffered_chan.h included multiple times
 * to create implementations of hgl_buffered_chan for different types:
 *
 *     #define HGL_BUFFERED_CHAN_TYPE char *
 *     #define HGL_BUFFERED_CHAN_TYPE_ID charp
 *     #include "hgl_buffered_chan.h"
 *
 *     #undef HGL_BUFFERED_CHAN_TYPE
 *     #undef HGL_BUFFERED_CHAN_TYPE_ID
 *     #define HGL_BUFFERED_CHAN_TYPE int
 *     #define HGL_BUFFERED_CHAN_TYPE_ID int
 *     #include "hgl_buffered_chan.h"
 *
 * If HGL_BUFFERED_CHAN_TYPE and HGL_BUFFERED_CHAN_TYPE_ID are left undefined, the default element type of
 * hgl_buffered_chan will be void *, and the default element type identifier will be "voidp".
 *
 * hgl_buffered_chan allows the default allocator and corresponding free function to be overridden by
 * redefining the following defines before including hgl_buffered_chan.h:
 *
 *     #define HGL_BUFFERED_CHAN_ALLOCATOR                (malloc)
 *     #define HGL_BUFFERED_CHAN_FREE                     (free)
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

/* CONFIGURABLE: HGL_BUFFERED_CHAN_TYPE & HGL_BUFFERED_CHAN_TYPE_ID */
#ifndef HGL_BUFFERED_CHAN_TYPE
#define HGL_BUFFERED_CHAN_TYPE void *
#define HGL_BUFFERED_CHAN_TYPE_ID voidp
#endif /* HGL_BUFFERED_CHAN_TYPE */

/* CONFIGURABLE: HGL_BUFFERED_CHAN_ALLOCATOR, HGL_BUFFERED_CHAN_REALLOCATOR, HGL_BUFFERED_CHAN_FREE */
#if !defined(HGL_BUFFERED_CHAN_ALLOCATOR) && !defined(HGL_BUFFERED_CHAN_FREE)
#define HGL_BUFFERED_CHAN_ALLOCATOR (malloc)
#define HGL_BUFFERED_CHAN_FREE (free)
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_BUFFERED_CHAN_STRUCT _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_t)
typedef struct
{
    HGL_BUFFERED_CHAN_TYPE *buf;
    uint32_t read_offset;
    uint32_t write_offset;
    uint32_t capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_writable;
    pthread_cond_t cvar_readable;
    int efd;
} HGL_BUFFERED_CHAN_STRUCT;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Channel functions -----------------------------------------------------------------*/

#define HGL_BUFFERED_CHAN_FUNC_INIT _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_init)
static inline void HGL_BUFFERED_CHAN_FUNC_INIT(HGL_BUFFERED_CHAN_STRUCT *chan, size_t capacity)
{
    HGL_ASSERT((capacity & (capacity -1)) == 0, "buffered chan capacity must be a power of 2");
    chan->buf = HGL_BUFFERED_CHAN_ALLOCATOR(sizeof(HGL_BUFFERED_CHAN_TYPE) * capacity);
    chan->read_offset = 0;
    chan->write_offset = 0;
    chan->capacity = capacity;
    pthread_mutex_init(&chan->mutex, NULL);
    pthread_cond_init(&chan->cvar_writable, NULL);
    pthread_cond_init(&chan->cvar_readable, NULL);
    chan->efd = eventfd(0, EFD_SEMAPHORE);
}

#define HGL_BUFFERED_CHAN_FUNC_FREE _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_free)
static inline void HGL_BUFFERED_CHAN_FUNC_FREE(HGL_BUFFERED_CHAN_STRUCT *chan)
{
    pthread_mutex_destroy(&chan->mutex);
    pthread_cond_destroy(&chan->cvar_writable);
    pthread_cond_destroy(&chan->cvar_readable);
    HGL_BUFFERED_CHAN_FREE(chan->buf);
    close(chan->efd);
}

#define HGL_BUFFERED_CHAN_FUNC_SEND _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_send)
static inline void HGL_BUFFERED_CHAN_FUNC_SEND(HGL_BUFFERED_CHAN_STRUCT *chan, 
                                               HGL_BUFFERED_CHAN_TYPE *item)
{
    pthread_mutex_lock(&chan->mutex);
    /* while not writable... */
    while(((chan->write_offset + 1) & (chan->capacity - 1)) == chan->read_offset) {
        pthread_cond_wait(&chan->cvar_writable, &chan->mutex);
    }
    chan->buf[chan->write_offset] = *item;
    chan->write_offset = (chan->write_offset + 1) & (chan->capacity - 1);
    uint64_t efd_inc = 1;
    write(chan->efd, &efd_inc, sizeof(uint64_t)); // increment counter
    pthread_cond_signal(&chan->cvar_readable);
    pthread_mutex_unlock(&chan->mutex);
}

#define HGL_BUFFERED_CHAN_FUNC_SEND_VALUE _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_send_value)
static inline void HGL_BUFFERED_CHAN_FUNC_SEND_VALUE(HGL_BUFFERED_CHAN_STRUCT *chan, 
                                                     HGL_BUFFERED_CHAN_TYPE item)
{
    HGL_BUFFERED_CHAN_FUNC_SEND(chan, &item);
}

#define HGL_BUFFERED_CHAN_FUNC_RECV _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_recv)
static inline HGL_BUFFERED_CHAN_TYPE HGL_BUFFERED_CHAN_FUNC_RECV(HGL_BUFFERED_CHAN_STRUCT *chan)
{
    pthread_mutex_lock(&chan->mutex);
    /* while not readable... */
    while(chan->write_offset == chan->read_offset) {
        pthread_cond_wait(&chan->cvar_readable, &chan->mutex);
    }
    HGL_BUFFERED_CHAN_TYPE item = chan->buf[chan->read_offset];
    chan->read_offset = (chan->read_offset + 1) & (chan->capacity - 1);
    uint64_t efd_counter;
    read(chan->efd, &efd_counter, sizeof(uint64_t)); // decrement counter
    pthread_cond_signal(&chan->cvar_writable);
    pthread_mutex_unlock(&chan->mutex);
    return item;
}

#define HGL_BUFFERED_CHAN_FUNC_SELECT _CONCAT3(hgl_, HGL_BUFFERED_CHAN_TYPE_ID, _buffered_chan_select)
static inline HGL_BUFFERED_CHAN_STRUCT *HGL_BUFFERED_CHAN_FUNC_SELECT(int n_args, ...)
{
    HGL_BUFFERED_CHAN_STRUCT *ret = NULL;

    /* setup va_list */
    va_list args1, args2;    
    va_start(args1, n_args);
    va_copy(args2, args1);

    /* populate pfds */
    struct pollfd *pfds = HGL_BUFFERED_CHAN_ALLOCATOR(n_args * sizeof(struct pollfd));
    if (pfds == NULL) {
        goto out;
    }
    for (int i = 0; i < n_args; i++) {
        HGL_BUFFERED_CHAN_STRUCT *c = va_arg(args1, HGL_BUFFERED_CHAN_STRUCT *);
        pfds[i].fd = c->efd;
        pfds[i].events = POLLIN;
    }

    /* poll: wait (forever) till one of the channels becomes readable */
    poll(pfds, (nfds_t) n_args, -1);

    /* find the first readable channel */
    for (int i = 0; i < n_args; i++) {
        HGL_BUFFERED_CHAN_STRUCT *c = va_arg(args2, HGL_BUFFERED_CHAN_STRUCT *);
        if (c->write_offset != c->read_offset /* readble */) {
            ret = c;
            break;
        }
    }
    
    HGL_BUFFERED_CHAN_FREE(pfds);
out:
    va_end(args1);
    va_end(args2);
    return ret;
}

