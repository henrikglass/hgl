
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
 * hgl_q.h implements a general purpose minimal macro-only synchronized (thread) queue.
 *
 *
 * USAGE:
 *
 * Just create a struct of the correct form by using the HglThreadQueue(T, N) macro:
 *
 *     typedef HglThreadQueue(float, 256) FloatQueue;
 *
 * To create:
 *
 *     FloatQueue q;
 *     hgl_tq_init(&q);
 *
 * To push an element:
 *
 *     hgl_tq_push(&q, 123.456f);
 *
 * To pop an element:
 *
 *     float element;
 *     element = hgl_tq_pop(&q, float);
 *
 * To wait for the receiver to become idle (waiting on empty queue):
 *
 *     hgl_tq_wait_until_idle(&q);
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_TQ_H
#define HGL_TQ_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define HGL_TQ_ARR_DECL(T, N) T arr[(N > 1 && N <= UINT16_MAX) ? N : -1] // Assert that N is in the range [2, 2^16]

#define HglThreadQueue(T, N)          \
    struct                            \
    {                                 \
        HGL_TQ_ARR_DECL_ASSERT(T, N); \
        pthread_mutex_t mutex;        \
        pthread_cond_t cvar_writable; \
        pthread_cond_t cvar_readable; \
        uint16_t wp;                  \
        uint16_t rp;                  \
        int n_idle;                   \
    }


#define hgl_tq_capacity(q) (sizeof((q)->arr) / sizeof((q)->arr[0]))
#define hgl_tq_is_empty(q) ((q)->rp == (q)->wp)
#define hgl_tq_is_full(q) ((((q)->wp + 1) & (hgl_tq_capacity(q) - 1)) == (q)->rp)

#define hgl_tq_init(q)                                             \
    do {                                                           \
        (q)->wp = 0;                                               \
        (q)->rp = 0;                                               \
        (q)->n_idle = 0;                                           \
        assert(0 == pthread_mutex_init(&(q)->mutex, NULL));        \
        assert(0 == pthread_cond_init(&(q)->cvar_writable, NULL)); \
        assert(0 == pthread_cond_init(&(q)->cvar_readable, NULL)); \
    } while (0)

#define hgl_tq_destroy(q)                                          \
    do {                                                           \
        pthread_mutex_lock(&(q)->mutex);                           \
        pthread_mutex_unlock(&(q)->mutex);                         \
        assert(0 == pthread_mutex_destroy(&(q)->mutex));           \
        assert(0 == pthread_cond_destroy(&(q)->cvar_writable));    \
        assert(0 == pthread_cond_destroy(&(q)->cvar_readable));    \
    } while (0)

#define hgl_tq_push(q, item)                                       \
    do {                                                           \
        pthread_mutex_lock(&(q)->mutex);                           \
        while(hgl_tq_is_full(q)) {                                 \
            pthread_cond_wait(&(q)->cvar_writable, &(q)->mutex);   \
        }                                                          \
        (q)->arr[(q)->wp] = item;                                  \
        (q)->wp = ((q)->wp + 1) & (hgl_tq_capacity(q) - 1);        \
        pthread_cond_signal(&(q)->cvar_readable);                  \
        pthread_mutex_unlock(&(q)->mutex);                         \
    } while (0)

#define hgl_tq_pop(q, T)                                           \
    ({                                                             \
        pthread_mutex_lock(&(q)->mutex);                           \
        while(hgl_tq_is_empty(q)) {                                \
            (q)->n_idle++;                                         \
            pthread_cond_wait(&(q)->cvar_readable, &(q)->mutex);   \
            (q)->n_idle--;                                         \
        }                                                          \
        T item = (q)->arr[(q)->rp];                                \
        (q)->rp = ((q)->rp + 1) & (hgl_tq_capacity(q) - 1);        \
        pthread_cond_signal(&(q)->cvar_writable);                  \
        pthread_mutex_unlock(&(q)->mutex);                         \
        item;                                                      \
    })

#define hgl_tq_wait_until_empty(q)                                 \
    do {                                                           \
        pthread_mutex_lock(&(q)->mutex);                           \
        while(!hgl_tq_is_empty(q)) {                               \
            pthread_cond_wait(&(q)->cvar_writable, &(q)->mutex);   \
        }                                                          \
        pthread_mutex_unlock(&(q)->mutex);                         \
    } while (0)

#define hgl_tq_wait_until_idle(q, n)                               \
    do {                                                           \
        hgl_tq_wait_until_empty(q);                                \
        while((q)->n_idle < n);                                    \
    } while (0)

#endif /* HGL_TQ_H */

