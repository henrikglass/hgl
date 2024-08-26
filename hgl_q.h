
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
 * hgl_q.h implements a general purpose minimal macro-only queue (circular buffer).
 *
 *
 * USAGE:
 *
 * Just create a struct of the correct form by using the HglDynamicArray(T) macro:
 *
 *     typedef HglQueue(float, 256) FloatQueue;
 *     FloatQueue q = {0};
 *     if (hgl_q_push_back(&q, 123.456f) != 0) {
 *         // push failed
 *     }
 *     // push ok
 *
 *     or:
 *
 *     HglQueue(float, 256) q = {0};
 *     hgl_q_push_back(&q, 123.456f);
 *
 * To pop an element:
 *
 *     float element;
 *     if (hgl_q_pop_front(&q, &element) != 0) {
 *         // pop failed.
 *     }
 *     // pop ok.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_Q_H
#define HGL_Q_H

#include <assert.h>

#define HglQueue(T, N)  \
    struct {            \
        T arr[N];       \
        size_t rp;      \
        size_t wp;      \
        size_t n_items; \
    }

#define hgl_q_pop_front(q, item)            \
    ({                                      \
        int ret = 0;                        \
        if (hgl_q_is_empty((q))) {          \
            ret = -1;                       \
        } else {                            \
            *item = (q)->arr[(q)->rp];      \
            (q)->rp++;                      \
            (q)->rp %= hgl_q_capacity((q)); \
            (q)->n_items--;                 \
        }                                   \
        ret;                                \
    })

#define hgl_q_push_back(q, item)            \
    ({                                      \
        int ret = 0;                        \
        if (hgl_q_is_full((q))) {           \
            ret = -1;                       \
        } else {                            \
            (q)->arr[(q)->wp] = item;       \
            (q)->wp++;                      \
            (q)->wp %= hgl_q_capacity((q)); \
            (q)->n_items++;                 \
        }                                   \
        ret;                                \
    })

#define hgl_q_is_full(q) ((q)->n_items == hgl_q_capacity((q)))
#define hgl_q_is_empty(q) ((q)->n_items == 0)
#define hgl_q_capacity(q) (sizeof((q)->arr) / sizeof((q)->arr[0]))

#define hgl_q_print(q) \
    do { \
        printf("[");\
        for (size_t i = 0; i < hgl_q_capacity((q)); i++) {\
            printf("%d, ", (q)->arr[i]); \
        } \
        printf("\b\b]\n");\
    } while(0)

#endif
