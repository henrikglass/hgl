
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
 * hgl_buffered_chan.h implements a thread-safe buffered message queue (FIFO) 
 * similar to buffered channels in go.
 *
 *
 * USAGE:
 *
 * Include hgl_buffered_chan.h file like this:
 *
 *     #define HGL_BUFFERED_CHAN_IMPLEMENTATION
 *     #include "hgl_buffered_chan.h"
 *
 * The following macros may be redefined if you wish to supply your own allocator:
 *
 *     #define HGL_BUFFERED_CHAN_ALLOC   malloc
 *     #define HGL_BUFFERED_CHAN_FREE    free
 *
 *
 * EXAMPLE:
 * 
 * See the examples directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_BUFFERED_CHAN_H
#define HGL_BUFFERED_CHAN_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <stdarg.h>

typedef struct
{
    void **items;
    bool dynamically_allocated_items;
    uint32_t read_offset;
    uint32_t write_offset;
    uint32_t capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cvar_writable;
    pthread_cond_t cvar_readable;
    int efd;
} HglBufferedChan;

/**
 * Creates a buffered channel with capacity `capacity`.
 */
HglBufferedChan hgl_buffered_chan_make(size_t capacity);

/**
 * Creates a buffered channel from a user-supplied buffer `buf` of size `size`.
 */
HglBufferedChan hgl_buffered_chan_make_from_buffer(void *buf, size_t size);

/**
 * Destroys the buffered channel `c`.
 */
void hgl_buffered_chan_destroy(HglBufferedChan *c);

/**
 * Sends item `item` on the buffered channel `c`. Will block if the queue in `c` is full.
 */
void hgl_buffered_chan_send(HglBufferedChan *c, void *item);

/**
 * Tries to send item `item` on the buffered channel `c`. If the internal queue of `c` is
 * full then `item` will not be sent and -1 will be returned. Otherwise, the item is sent,
 * and 0 is returned.
 */
int hgl_buffered_chan_try_send(HglBufferedChan *c, void *item);

/**
 * Recieve an item from the channel `c`. Will block until there is at
 * least one item on `c`.
 */
void *hgl_buffered_chan_recv(HglBufferedChan *c);

/**
 * Tries to receive an item from the channel `c`. If the internal queue of `c` is empty
 * then NULL will be returned. Otherwise, the next item will be returned.
 */
void *hgl_buffered_chan_try_recv(HglBufferedChan *c);

/**
 * Wait on any number (<= 128) of channels simultaneously, until at least one
 * of them becomes readable (i.e. something was sent on the channel). Returns
 * a pointer to the first readable channel in the variadic arguments list.
 */
HglBufferedChan *hgl_buffered_chan_select(int n_args, ...);

/**
 * Returns a pointer to the first readable channel in the variadic arguments 
 * list. If there are no readable channels NULL is returned.
 */
HglBufferedChan *hgl_buffered_chan_try_select(int n_args, ...);

#endif /* HGL_BUFFERED_CHAN_H  */

/*--- Public variables ------------------------------------------------------------------*/

/*--- Channel functions -----------------------------------------------------------------*/

#ifdef HGL_BUFFERED_CHAN_IMPLEMENTATION

/* CONFIGURABLE: HGL_BUFFERED_CHAN_ALLOC, HGL_BUFFERED_CHAN_FREE */
#if !defined(HGL_BUFFERED_CHAN_ALLOC) && !defined(HGL_BUFFERED_CHAN_FREE)
#include <stdlib.h>
#define HGL_BUFFERED_CHAN_ALLOC  malloc
#define HGL_BUFFERED_CHAN_FREE   free
#endif

#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

HglBufferedChan hgl_buffered_chan_make(size_t capacity)
{
    int err;
    HglBufferedChan c = (HglBufferedChan) {.items = NULL, .capacity = 0};
    if ((capacity & (capacity -1)) != 0) {
        fprintf(stderr, "[hgl_buffered_chan.h] Error: buffered chan capacity must be a power of 2\n");
        return c;
    }
    c.items = HGL_BUFFERED_CHAN_ALLOC(sizeof(c.items) * capacity);
    if (c.items == NULL) {
        fprintf(stderr, "[hgl_chan.h]: Failed to create channel.\n");
        return c;
    }
    c.dynamically_allocated_items = true;
    c.read_offset = 0;
    c.write_offset = 0;
    c.capacity = capacity;
    err  = pthread_mutex_init(&c.mutex, NULL);
    err |= pthread_cond_init(&c.cvar_writable, NULL);
    err |= pthread_cond_init(&c.cvar_readable, NULL);
    c.efd = eventfd(0, EFD_SEMAPHORE);
    if (err != 0) {
        fprintf(stderr, "[hgl_chan.h]: Failed to create channel.\n");
        HGL_BUFFERED_CHAN_FREE(c.items);
    }
    return c;
}

HglBufferedChan hgl_buffered_chan_make_from_buffer(void *buf, size_t size)
{
    int err;
    HglBufferedChan c = (HglBufferedChan) {.items = NULL, .capacity = 0};
    uint32_t capacity = size / sizeof(void *);
    if ((capacity & (capacity -1)) != 0) {
        fprintf(stderr, "[hgl_buffered_chan.h] Error: buffered chan capacity must be a power of 2\n");
        return c;
    }
    c.items = buf;
    c.dynamically_allocated_items = false;
    c.read_offset = 0;
    c.write_offset = 0;
    c.capacity = capacity;
    err  = pthread_mutex_init(&c.mutex, NULL);
    err |= pthread_cond_init(&c.cvar_writable, NULL);
    err |= pthread_cond_init(&c.cvar_readable, NULL);
    c.efd = eventfd(0, EFD_SEMAPHORE);
    if (err != 0) {
        fprintf(stderr, "[hgl_chan.h]: Failed to create channel.\n");
        HGL_BUFFERED_CHAN_FREE(c.items);
    }
    return c;
}

void hgl_buffered_chan_destroy(HglBufferedChan *c)
{
    pthread_mutex_destroy(&c->mutex);
    pthread_cond_destroy(&c->cvar_writable);
    pthread_cond_destroy(&c->cvar_readable);
    if (c->dynamically_allocated_items) {
        HGL_BUFFERED_CHAN_FREE(c->items);
    }
    close(c->efd);
}

void hgl_buffered_chan_send(HglBufferedChan *c, void *item)
{
    pthread_mutex_lock(&c->mutex);
    /* while not writable... */
    while(((c->write_offset + 1) & (c->capacity - 1)) == c->read_offset) {
        pthread_cond_wait(&c->cvar_writable, &c->mutex);
    }
    c->items[c->write_offset] = item;
    c->write_offset = (c->write_offset + 1) & (c->capacity - 1);
    uint64_t efd_inc = 1;
    write(c->efd, &efd_inc, sizeof(uint64_t)); // increment counter
    pthread_cond_signal(&c->cvar_readable);
    pthread_mutex_unlock(&c->mutex);
}

int hgl_buffered_chan_try_send(HglBufferedChan *c, void *item)
{
    pthread_mutex_lock(&c->mutex);
    /* while not writable... */
    while(((c->write_offset + 1) & (c->capacity - 1)) == c->read_offset) {
        pthread_mutex_unlock(&c->mutex);
        return -1;
    }
    c->items[c->write_offset] = item;
    c->write_offset = (c->write_offset + 1) & (c->capacity - 1);
    uint64_t efd_inc = 1;
    write(c->efd, &efd_inc, sizeof(uint64_t)); // increment counter
    pthread_cond_signal(&c->cvar_readable);
    pthread_mutex_unlock(&c->mutex);
    return 0;
}

void *hgl_buffered_chan_recv(HglBufferedChan *c)
{
    pthread_mutex_lock(&c->mutex);
    /* while not readable... */
    while(c->write_offset == c->read_offset) {
        pthread_cond_wait(&c->cvar_readable, &c->mutex);
    }
    void *item = c->items[c->read_offset];
    c->read_offset = (c->read_offset + 1) & (c->capacity - 1);
    uint64_t efd_counter;
    read(c->efd, &efd_counter, sizeof(uint64_t)); // decrement counter
    pthread_cond_signal(&c->cvar_writable);
    pthread_mutex_unlock(&c->mutex);
    return item;
}

void *hgl_buffered_chan_try_recv(HglBufferedChan *c)
{
    pthread_mutex_lock(&c->mutex);
    /* while not readable... */
    while(c->write_offset == c->read_offset) {
        pthread_mutex_unlock(&c->mutex);
        return NULL;
    }
    void *item = c->items[c->read_offset];
    c->read_offset = (c->read_offset + 1) & (c->capacity - 1);
    uint64_t efd_counter;
    read(c->efd, &efd_counter, sizeof(uint64_t)); // decrement counter
    pthread_cond_signal(&c->cvar_writable);
    pthread_mutex_unlock(&c->mutex);
    return item;
}

HglBufferedChan *hgl_buffered_chan_select(int n_args, ...)
{
    HglBufferedChan *ret = NULL;
    static struct pollfd pfds[128]; // 128 should be more than enough

    /* setup va_list */
    va_list args1, args2;    
    va_start(args1, n_args);
    va_copy(args2, args1);

    /* populate pfds */
    for (int i = 0; i < n_args; i++) {
        HglBufferedChan *c = va_arg(args1, HglBufferedChan *);
        pfds[i].fd = c->efd;
        pfds[i].events = POLLIN;
    }

    /* poll: wait (forever) till one of the channels becomes readable */
    poll(pfds, (nfds_t) n_args, -1);

    /* find the first readable channel */
    for (int i = 0; i < n_args; i++) {
        HglBufferedChan *c = va_arg(args2, HglBufferedChan *);
        if (c->write_offset != c->read_offset /* readble */) {
            ret = c;
            break;
        }
    }
    
    va_end(args1);
    va_end(args2);
    return ret;
}

HglBufferedChan *hgl_buffered_chan_try_select(int n_args, ...)
{
    HglBufferedChan *ret = NULL;

    /* setup va_list */
    va_list args;    
    va_start(args, n_args);

    for (int i = 0; i < n_args; i++) {
        HglBufferedChan *c = va_arg(args, HglBufferedChan *);
        if (c->write_offset != c->read_offset /* readble */) {
            ret = c;
            break;
        }  
    }

    va_end(args);
    return ret;
}

#endif

