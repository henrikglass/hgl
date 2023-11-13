
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
 * hgl_chan.h implements a thread-safe message queue similar to go channels.
 *
 *
 * USAGE:
 *
 * Include hgl_chan.h file like this:
 *
 *     #define HGL_CHAN_TYPE char *
 *     #define HGL_CHAN_TYPE_ID charp
 *     #include "hgl_chan.h"
 *
 * This will create an implementation of hgl_chan capable of holding an element of type char *.
 * "charp" is used as an infix in the identifiers exposed by the library. Below is a complete
 * list of the generated API:
 *
 *     void hgl_charp_chan_init(hgl_charp_chan_t *chan);
 *     void hgl_charp_chan_free(hgl_charp_chan_t *chan);
 *     void hgl_charp_chan_send(hgl_charp_chan_t *chan, char **item);
 *     void hgl_charp_chan_send_value(hgl_charp_chan_t *chan, char *item);
 *     char *hgl_charp_chan_recv(hgl_charp_chan_t *chan);
 *     hgl_charp_chan_t *hgl_charp_chan_select(int n_args, ...);
 *
 * HGL_CHAN_TYPE and HGL_CHAN_TYPE_ID may be redefined and hgl_chan.h included multiple times
 * to create implementations of hgl_chan for different types:
 *
 *     #define HGL_CHAN_TYPE char *
 *     #define HGL_CHAN_TYPE_ID charp
 *     #include "hgl_chan.h"
 *
 *     #undef HGL_CHAN_TYPE
 *     #undef HGL_CHAN_TYPE_ID
 *     #define HGL_CHAN_TYPE int
 *     #define HGL_CHAN_TYPE_ID int
 *     #include "hgl_chan.h"
 *
 * If HGL_CHAN_TYPE and HGL_CHAN_TYPE_ID are left undefined, the default element type of
 * hgl_chan will be void *, and the default element type identifier will be "voidp".
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Helper macros ---------------------------------------------------------------------*/

#define _CONCAT_NX3(a, b, c) a ## b ## c
#define _CONCAT3(a, b, c) _CONCAT_NX3(a, b, c)

/*--- channel-specific macros -----------------------------------------------------------*/

/* CONFIGURABLE: HGL_CHAN_TYPE & HGL_CHAN_TYPE_ID */
#ifndef HGL_CHAN_TYPE
#define HGL_CHAN_TYPE void *
#define HGL_CHAN_TYPE_ID voidp
#endif /* HGL_CHAN_TYPE */

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

/*--- Public type definitions -----------------------------------------------------------*/

#define HGL_CHAN_STRUCT_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_t)
typedef struct
{
    HGL_CHAN_TYPE item;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_writable;
    pthread_cond_t cvar_readable;
    bool readable;
    int efd;
} HGL_CHAN_STRUCT_NAME;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Channel functions -----------------------------------------------------------------*/

#define HGL_CHAN_FUNC_INIT_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_init)
static inline void HGL_CHAN_FUNC_INIT_NAME(HGL_CHAN_STRUCT_NAME *chan)
{
    chan->readable = false;
    pthread_mutex_init(&chan->mutex, NULL);
    pthread_cond_init(&chan->cvar_writable, NULL);
    pthread_cond_init(&chan->cvar_readable, NULL);
    chan->efd = eventfd(0,0);
}

#define HGL_CHAN_FUNC_FREE_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_free)
static inline void HGL_CHAN_FUNC_FREE_NAME(HGL_CHAN_STRUCT_NAME *chan)
{
    pthread_mutex_destroy(&chan->mutex);
    pthread_cond_destroy(&chan->cvar_writable);
    pthread_cond_destroy(&chan->cvar_readable);
    close(chan->efd);
}

#define HGL_CHAN_FUNC_SEND_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_send)
static inline void HGL_CHAN_FUNC_SEND_NAME(HGL_CHAN_STRUCT_NAME *chan, HGL_CHAN_TYPE *item)
{
    pthread_mutex_lock(&chan->mutex);
    while(chan->readable /* impl. not writable */) {
        pthread_cond_wait(&chan->cvar_writable, &chan->mutex);
    }
    chan->item = *item;
    chan->readable = true;
    uint64_t efd_counter = 1;
    write(chan->efd, &efd_counter, 8);
    pthread_cond_signal(&chan->cvar_readable);
    pthread_mutex_unlock(&chan->mutex);
}

#define HGL_CHAN_FUNC_SEND_VALUE_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_send_value)
static inline void HGL_CHAN_FUNC_SEND_VALUE_NAME(HGL_CHAN_STRUCT_NAME *chan, HGL_CHAN_TYPE item)
{
    HGL_CHAN_FUNC_SEND_NAME(chan, &item);
}

#define HGL_CHAN_FUNC_RECV_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_recv)
static inline HGL_CHAN_TYPE HGL_CHAN_FUNC_RECV_NAME(HGL_CHAN_STRUCT_NAME *chan)
{
    pthread_mutex_lock(&chan->mutex);
    while(!chan->readable) {
        pthread_cond_wait(&chan->cvar_readable, &chan->mutex);
    }
    HGL_CHAN_TYPE item = chan->item;
    chan->readable = false;
    uint64_t efd_counter;
    read(chan->efd, &efd_counter, 8); // resets counter to zero
    pthread_cond_signal(&chan->cvar_writable);
    pthread_mutex_unlock(&chan->mutex);
    return item;
}

#define HGL_CHAN_FUNC_SELECT_NAME _CONCAT3(hgl_, HGL_CHAN_TYPE_ID, _chan_select)
static inline HGL_CHAN_STRUCT_NAME *HGL_CHAN_FUNC_SELECT_NAME(int n_args, ...)
{
    /* setup va_list & pfds */
    HGL_CHAN_STRUCT_NAME *ret = NULL;
    struct pollfd *pfds = malloc(n_args * sizeof(struct pollfd));
    va_list args1, args2;    
    va_start(args1, n_args);
    va_copy(args2, args1);

    /* populate pfds */
    for (int i = 0; i < n_args; i++) {
        HGL_CHAN_STRUCT_NAME *c = va_arg(args1, HGL_CHAN_STRUCT_NAME *);
        pfds[i].fd = c->efd;
        pfds[i].events = POLLIN;
    }

    /* poll: wait (forever) till one of the channels becomes readable */
    poll(pfds, (unsigned long) n_args, -1);

    /* find the readable channel */
    for (int i = 0; i < n_args; i++) {
        HGL_CHAN_STRUCT_NAME *c = va_arg(args2, HGL_CHAN_STRUCT_NAME *);
        if (c->readable) {
            ret = c;
            break;
        }
    }

    /* cleanup and return the readable channel */
    va_end(args1);
    va_end(args2);
    free(pfds);
    return ret;
}
