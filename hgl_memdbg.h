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
 * hgl_memdbg.h implements a simple leak checking utility.
 *
 *
 * USAGE:
 *
 * Include hgl_memdbg.h file like this:
 *
 *     #define HGL_MEMDBG_IMPLEMENTATION
 *     #include "hgl_memdbg.h"
 *
 * HGL_MEMDBG_IMPLEMENTATION must only be defined once, in a single compilation unit.
 * hgl_memdbg redefines 'malloc', 'realloc', and 'free' as macros to internal functions
 * that keep track of which allocations are made where in the code. To use hgl_memdbg,
 * simply include the header, as shown above, and call 'hgl_memdbg_report' on exit.
 *
 * hgl_memdbg.h keeps track of memory by maintaining a doubly linked list of all
 * allocations. Each linked list node is allocated together with the memory requested
 * in a call to `malloc` (or `realloc`). I.e., if the user requests 8 bytes of memory
 * then the internal hgl_memdbg implementation will request 16 + 8 bytes, store the
 * linked list node in the top 16 bytes, and return a pointer to the remaining 8.
 *
 * Optionally, you can define the following:
 *
 *     #define HGL_MEMDBG_ENABLE_STACKTRACES
 *
 * HGL_MEMDBG_ENABLE_STACKTRACES will store generate a stack trace for each call to
 * malloc and realloc that will be printed in case of a memory leak. Note that this
 * will cause some overhead for calls to malloc, realloc, and free.
 *
 * To disable hgl_memdbg.h without having to comment out all the 
 * `#include "hgl_memdbg.h"` in your code, you can define the following macro
 * ahead of including the implementation:
 *
 *     #define HGL_MEMDBG_DISABLE
 *
 * EXAMPLE:
 *
 * This program will report that there was a leak of 512 bytes on line 8 and return
 * with exit code 1:
 *
 *     #define HGL_MEMDBG_ENABLE_STACKTRACES
 *     #define HGL_MEMDBG_IMPLEMENTATION
 *     #include "hgl_memdbg.h"
 *
 *     int main(void)
 *     {
 *
 *         double *d = malloc(sizeof(double));
 *         double *arr = malloc(64 * sizeof(double)); // line 8
 *         free(d);
 *
 *         // Oops, we forgot to free 'arr'!
 *
 *         // NB: Optional! hgl_memdbg.h will automatically register 
 *         //     hgl_memdbg_report() to run upon program exit.
 *         // int err = hgl_memdbg_report();
 *         // return (err != 0) ? 1 : 0;
 *     }
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_MEMDBG_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*--- Public macros ---------------------------------------------------------------------*/

#define HGL_RED     "\x1b[31m"
#define HGL_AMBER   "\x1b[33m"
#define HGL_MAGENTA "\x1b[35m"
#define HGL_NC      "\x1b[0m"

/*--- Public type definitions -----------------------------------------------------------*/

/* Goes at the start of an allocation */
// TODO reorder largest --> smallest
typedef struct HglAllocationHeader {
    size_t size;
    struct HglAllocationHeader *prev;
    struct HglAllocationHeader *next;
    const char *expr_str;
    const char *file;
    int line;
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
    char **stack_trace;
    int stack_trace_depth;
#endif
} HglAllocationHeader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void *hgl_memdbg_internal_malloc_(size_t size, const char *file, int line, const char *expr_str);
void *hgl_memdbg_internal_calloc_(size_t nmemb, size_t size, const char *file, int line, const char *expr_str);
void *hgl_memdbg_internal_realloc_(void *ptr, size_t size, const char *file, int line, const char *expr_str);
void hgl_memdbg_internal_free_(void *ptr);

/**
 * Prints a memory leak report.
 */
int hgl_memdbg_report(void);

void hgl_memdbg_atexit_(void);

#endif /* HGL_MEMDBG_H */

#ifdef HGL_MEMDBG_IMPLEMENTATION

#include <stdio.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
#include <execinfo.h>
#endif

/* Size of header + required padding to keep the alignment constraints of malloc */
#define HGL_MEMDBG_PADDED_HEADER_SIZE (sizeof(HglAllocationHeader) + (_Alignof(max_align_t) - \
                                      (sizeof(HglAllocationHeader) & (_Alignof(max_align_t) - 1))))


static HglAllocationHeader *hgl_allocation_header_head_ = NULL;
static pthread_mutex_t hgl_memdbg_mutex_ = PTHREAD_MUTEX_INITIALIZER;
static bool exit_func_registered = false;

void *hgl_memdbg_internal_malloc_(size_t size, const char *file, int line, const char *expr_str)
{
#ifdef HGL_MEMDBG_DISABLE
    (void) file;
    (void) line;
    (void) expr_str;
    return malloc(size);
#endif

    if (!exit_func_registered) {
        assert(0 == atexit(hgl_memdbg_atexit_));
        exit_func_registered = true;
    }

    if (size == 0) {
        return NULL;
    }

    HglAllocationHeader *header = malloc(HGL_MEMDBG_PADDED_HEADER_SIZE + size);

    /* Return NULL if malloc failed. */
    if (header == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&hgl_memdbg_mutex_);

    /* Construct header and insert into circular doubly-linked list */
    header->expr_str = expr_str;
    header->file     = file;
    header->line     = line;
    header->size     = size;
    if (hgl_allocation_header_head_ == NULL) {
        header->prev = header;
        header->next = header;
        hgl_allocation_header_head_ = header;
    } else {
        HglAllocationHeader *temp = hgl_allocation_header_head_->next;
        hgl_allocation_header_head_->next = header;
        header->next = temp;
        header->next->prev = header;
        header->prev = hgl_allocation_header_head_;
    }

#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
    /* Generate stack trace. */
    void *tmp[32];
    header->stack_trace_depth = backtrace(tmp, 32);
    header->stack_trace = backtrace_symbols(tmp, header->stack_trace_depth);
#endif

    pthread_mutex_unlock(&hgl_memdbg_mutex_);

    /* Return memory after header */
    return (void *)((uint8_t *)(header) + HGL_MEMDBG_PADDED_HEADER_SIZE);
}

void *hgl_memdbg_internal_calloc_(size_t nmemb, size_t size, const char *file, int line, const char *expr_str)
{
#ifdef HGL_MEMDBG_DISABLE
    (void) file;
    (void) line;
    (void) expr_str;
    return calloc(nmemb, size);
#endif

    if (!exit_func_registered) {
        assert(0 == atexit(hgl_memdbg_atexit_));
        exit_func_registered = true;
    }

    if (size == 0 || nmemb == 0) {
        return NULL;
    }

    void *ptr = hgl_memdbg_internal_malloc_(nmemb * size, file, line, expr_str);
    memset(ptr, 0, nmemb * size);

    return ptr;
}

void *hgl_memdbg_internal_realloc_(void *ptr, size_t size, const char *file, int line, const char *expr_str)
{
#ifdef HGL_MEMDBG_DISABLE
    (void) file;
    (void) line;
    (void) expr_str;
    return realloc(ptr, size);
#endif

    if (!exit_func_registered) {
        assert(0 == atexit(hgl_memdbg_atexit_));
        exit_func_registered = true;
    }

    if (ptr == NULL) {
        return hgl_memdbg_internal_malloc_(size, file, line, expr_str);
    }

    pthread_mutex_lock(&hgl_memdbg_mutex_);

    uint8_t *ptr8 = (uint8_t *) ptr;
    HglAllocationHeader *header = (HglAllocationHeader *) (ptr8 - HGL_MEMDBG_PADDED_HEADER_SIZE);
    HglAllocationHeader *prev = header->prev;
    HglAllocationHeader *next = header->next;
    bool is_head      = (header == hgl_allocation_header_head_);
    bool is_only_node = (header == next) && (header == prev);
    header = realloc((void *) header, size + HGL_MEMDBG_PADDED_HEADER_SIZE);
    if (header == NULL) {
        pthread_mutex_unlock(&hgl_memdbg_mutex_);
        return NULL;
    }

    /* Update neighbours' pointers */
    if (!is_only_node) {
        next->prev = header;
        prev->next = header;
    } else {
        header->next = header;
        header->prev = header;
    }

    /* Update header */
    header->expr_str = expr_str;
    header->size     = size;
    header->file     = file;
    header->line     = line;

#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
    /* Re-generate stack trace. */
    free(header->stack_trace);
    void *tmp[32];
    header->stack_trace_depth = backtrace(tmp, 32);
    header->stack_trace = backtrace_symbols(tmp, header->stack_trace_depth);
#endif

    /* update head if this node is the head */
    if (is_head) {
        hgl_allocation_header_head_ = header;
    }

    pthread_mutex_unlock(&hgl_memdbg_mutex_);

    return (void *)((uint8_t *)(header) + HGL_MEMDBG_PADDED_HEADER_SIZE);
}

void hgl_memdbg_internal_free_(void *ptr)
{
#ifdef HGL_MEMDBG_DISABLE
    free(ptr);
    return;
#endif

    if (!exit_func_registered) {
        assert(0 == atexit(hgl_memdbg_atexit_));
        exit_func_registered = true;
    }

    uint8_t *ptr8 = (uint8_t *) ptr;

    if (ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&hgl_memdbg_mutex_);

    /* Remove allocation from linked list and free it. */
    HglAllocationHeader *header = (HglAllocationHeader *) (ptr8 - HGL_MEMDBG_PADDED_HEADER_SIZE);
    header->prev->next = header->next;
    header->next->prev = header->prev;
    HglAllocationHeader *next = header->next;
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
    free(header->stack_trace);
#endif
    free((void *) header);

    /* If this header was the head of the linked list, make the next one head. */
    if (hgl_allocation_header_head_ == header) {
        hgl_allocation_header_head_ = next;
    }

    /*
     * If the next one is also the head, then there was only one allocation in the list.
     * Make head == NULL.
     */
    if (hgl_allocation_header_head_ == header) {
        hgl_allocation_header_head_ = NULL;
    }

    pthread_mutex_unlock(&hgl_memdbg_mutex_);
}

int hgl_memdbg_report()
{
    size_t total_unfreed = 0;
    HglAllocationHeader *header = hgl_allocation_header_head_;

#if 0
    int i = 0;
    if (header != NULL) {
        printf("HEAD: ");
        do {
            i++;
            printf("[%p - %p - %p] ----> ", (void *)header->prev, (void *) header, (void *)header->next);
            header = header->next;
        } while (header != hgl_allocation_header_head_ && i < 10);
        printf("\n");
    }
#endif

    printf("============================ [%shgl_memdbg report%s] ============================\n",
           HGL_MAGENTA, HGL_NC);
    if (header != NULL) {
        printf("\n");
        pthread_mutex_lock(&hgl_memdbg_mutex_);
        do {
            printf("[%s%s:%d%s]: Call to \"%s\" %sLEAKED%s %zu bytes of memory.\n",
                   HGL_AMBER, header->file, header->line, HGL_NC, header->expr_str,
                   HGL_RED, HGL_NC, header->size);
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
            if (header->stack_trace != NULL) {
                printf ("    Stack trace at allocation (compile with -rdynamic to enable symbols): \n");
                for (int i = 0; i < header->stack_trace_depth; i++) {
                    printf ("      [%d] %s\n", i, header->stack_trace[i]);
                }
            }
#endif
            total_unfreed += header->size;
            header = header->next;
        } while (header != hgl_allocation_header_head_);
        pthread_mutex_unlock(&hgl_memdbg_mutex_);
    }
    printf("\nTOTAL:\t%zu bytes left unfreed.\n\n", total_unfreed);
    printf("=============================================================================\n");

    return (total_unfreed != 0) ? -1 : 0;
}

void hgl_memdbg_atexit_(void)
{
    (void) hgl_memdbg_report();
}

#endif

#ifndef HGL_MEMDBG_H
#define HGL_MEMDBG_H

#define malloc(size)        (hgl_memdbg_internal_malloc_((size), __FILE__, __LINE__, "malloc(" #size ");"))
#define calloc(nmemb, size) (hgl_memdbg_internal_calloc_((nmemb), (size), __FILE__, __LINE__, "calloc(" #nmemb ", " #size ");"))
#define realloc(ptr, size)  (hgl_memdbg_internal_realloc_((ptr), (size), __FILE__, __LINE__, "realloc(" #ptr ", " #size ");"))
#define free(ptr)           (hgl_memdbg_internal_free_((ptr)))

#endif
