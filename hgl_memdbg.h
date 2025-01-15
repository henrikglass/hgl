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
 * To ensure that the linked list is maintained in a safe manner in a multithreaded
 * program, the user should define the following before including `hgl_memdbg.h`:
 *
 *     #define HGL_MEMDBG_ENABLE_MULTITHREAD
 *
 * `HGL_MEMDBG_ENABLE_MULTITHREAD` adds a global mutex to the program which is used
 * to ensure mutual exclusion of the entire linked list when accessed from different
 * threads.
 *
 * Optionally, you can define the following:
 *
 *     #define HGL_MEMDBG_ENABLE_STACKTRACES
 *
 * HGL_MEMDBG_ENABLE_STACKTRACES will store generate a stack trace for each call to
 * malloc and realloc that will be printed in case of a memory leak. Note that this
 * will cause some overhead for calls to malloc, realloc, and free.
 *
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
 *         int err = hgl_memdbg_report();
 *         return (err != 0) ? 1 : 0;
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
#define HGL_MAGENTA "\x1b[35m"
#define HGL_NC      "\x1b[0m"

/*--- Public type definitions -----------------------------------------------------------*/

/* Goes at the start of an allocation */
typedef struct HglAllocationHeader {
    const char *file;
    int line;
    size_t size;
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
    char **stack_trace;
    int stack_trace_depth;
#endif
    struct HglAllocationHeader *prev;
    struct HglAllocationHeader *next;
} HglAllocationHeader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void *hgl_memdbg_internal_malloc_(size_t size, const char *file, int line);
void *hgl_memdbg_internal_realloc_(void *ptr, size_t size, const char *file, int line);
void hgl_memdbg_internal_free_(void *ptr);

/**
 * Prints a memory leak report.
 *
 * Returns 0 if no leaks were found, -1 otherwise.
 */
int hgl_memdbg_report(void);

#endif /* HGL_MEMDBG_H */

#ifdef HGL_MEMDBG_IMPLEMENTATION

#include <stdio.h>
#include <stddef.h>

/* Size of header + required padding to keep the alignment constraints of malloc */
#define HGL_MEMDBG_PADDED_HEADER_SIZE (sizeof(HglAllocationHeader) + (_Alignof(max_align_t) - \
                                      (sizeof(HglAllocationHeader) & (_Alignof(max_align_t) - 1))))


HglAllocationHeader *hgl_allocation_header_head_ = NULL;

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
#include <pthread.h>
static pthread_mutex_t hgl_memdbg_mutex_ = PTHREAD_MUTEX_INITIALIZER;
#endif

void *hgl_memdbg_internal_malloc_(size_t size, const char *file, int line)
{

    if (size == 0) {
        return NULL;
    }

    HglAllocationHeader *header = malloc(HGL_MEMDBG_PADDED_HEADER_SIZE + size);

    /* Return NULL if malloc failed. */
    if (header == NULL) {
        return NULL;
    }

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_lock(&hgl_memdbg_mutex_);
#endif

    /* Construct header and insert into circular doubly-linked list */
    header->file = file;
    header->line = line;
    header->size = size;
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

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_unlock(&hgl_memdbg_mutex_);
#endif

    /* Return memory after header */
    return (void *)((uint8_t *)(header) + HGL_MEMDBG_PADDED_HEADER_SIZE);
}

void *hgl_memdbg_internal_realloc_(void *ptr, size_t size, const char *file, int line)
{
    if (ptr == NULL) {
        return hgl_memdbg_internal_malloc_(size, file, line);
    }

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_lock(&hgl_memdbg_mutex_);
#endif

    uint8_t *ptr8 = (uint8_t *) ptr;
    HglAllocationHeader *header = (HglAllocationHeader *) (ptr8 - HGL_MEMDBG_PADDED_HEADER_SIZE);
    HglAllocationHeader *prev = header->prev;
    HglAllocationHeader *next = header->next;
    bool is_head      = (header == hgl_allocation_header_head_);
    bool is_only_node = (header == next) && (header == prev);
    header = realloc((void *) header, size + HGL_MEMDBG_PADDED_HEADER_SIZE);
    if (header == NULL) {
#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
        pthread_mutex_unlock(&hgl_memdbg_mutex_);
#endif
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
    header->size = size;
    header->file = file;
    header->line = line;

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

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_unlock(&hgl_memdbg_mutex_);
#endif

    return (void *)((uint8_t *)(header) + HGL_MEMDBG_PADDED_HEADER_SIZE);
}

void hgl_memdbg_internal_free_(void *ptr)
{
    uint8_t *ptr8 = (uint8_t *) ptr;

    if (ptr == NULL) {
        return;
    }

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_lock(&hgl_memdbg_mutex_);
#endif

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

#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
    pthread_mutex_unlock(&hgl_memdbg_mutex_);
#endif
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

    printf("====================== [%shgl_memdbg report%s] ======================\n",
           HGL_MAGENTA, HGL_NC);
    if (header != NULL) {
        printf("\n");
#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
        pthread_mutex_lock(&hgl_memdbg_mutex_);
#endif
        do {
            printf("<%s:%d>: %sLEAKED%s %zu bytes of memory.\n",
                   header->file, header->line, HGL_RED, HGL_NC, header->size);
#ifdef HGL_MEMDBG_ENABLE_STACKTRACES
            if (header->stack_trace != NULL) {
                printf ("Stack trace at allocation (compile with -rdynamic to enable symbols): \n");
                for (int i = 0; i < header->stack_trace_depth; i++) {
                    printf ("  [%d] %s\n", i, header->stack_trace[i]);
                }
            }
#endif
            total_unfreed += header->size;
            header = header->next;
        } while (header != hgl_allocation_header_head_);
#ifdef HGL_MEMDBG_ENABLE_MULTITHREAD
        pthread_mutex_unlock(&hgl_memdbg_mutex_);
#endif
    }
    printf("\nTOTAL:\t%zu bytes left unfreed.\n\n", total_unfreed);
    printf("=================================================================\n");

    return (total_unfreed == 0) ? 0 : -1;
}

#endif

#ifndef HGL_MEMDBG_H
#define HGL_MEMDBG_H

#define malloc(size)        (hgl_memdbg_internal_malloc_((size), __FILE__, __LINE__))
#define realloc(ptr, size)  (hgl_memdbg_internal_realloc_((ptr), (size), __FILE__, __LINE__))
#define free(ptr)           (hgl_memdbg_internal_free_((ptr)))

#endif


