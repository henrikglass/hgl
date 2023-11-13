
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
 * hgl_stack_alloc.h implements a simple to use stack allocator (ontop of hgl_arena_alloc).
 *
 *
 * USAGE:
 *
 * Include hgl_stack_alloc.h file like this:
 *
 *     #define HGL_STACK_ALLOC_IMPLEMENTATION 
 *     #include "hgl_stack_alloc.h"
 *
 * This will create the implementation of hgl_stack_alloc in the current compilation unit. To
 * include hgl_stack_alloc.h without creating the implementation, simply ommit the #define 
 * of HGL_STACK_ALLOC_IMPLEMENTATION. Note that hgl_stack_alloc.h includes hgl_arena_alloc.h
 * internally, since it can be considered as an extension of hgl_arena_alloc.h and utilizes
 * some of its constructs. Make sure to copy both files into your project. When including
 * hgl_arena_alloc.h, hgl_stack_alloc will try to be clever about whether 
 * HGL_ARENA_ALLOC_IMPLEMENTATION is defined or not. 
 *
 * Below is a complet listing of the API:
 *
 * void *hgl_stack_alloc(HglArena *arena, size_t alloc_size);
 * void hgl_stack_free(HglArena *arena, void *ptr);
 *
 * Since hgl_stack_alloc builds ontop of hgl_arena_alloc, the alignment can be defined in the
 * same way as for hgl_arena_alloc:
 *
 *     #define HGL_ARENA_ALIGNMENT 64
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_STACK_ALLOC_H
#define HGL_STACK_ALLOC_H

/*--- stack alloc-specific macros -------------------------------------------------------*/

//#define HGL_STACK_ALLOC_DEBUG_PRINTS

/*--- Include files ---------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h> // memset

/* This should mostly behave ok */
#if !defined(HGL_ARENA_ALLOC_IMPLEMENTATION) && \
     defined(HGL_STACK_ALLOC_IMPLEMENTATION)
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#elif defined(HGL_ARENA_ALLOC_IMPLEMENTATION) 
#undef HGL_ARENA_ALLOC_IMPLEMENTATION
#endif
#include "hgl_arena_alloc.h"

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    size_t alloc_offset;
} HglStackFooter;

/**
 * Allocate a chunk of `alloc_size` bytes from `arena` using .
 */
void *hgl_stack_alloc(HglArena *arena, size_t alloc_size);

/**
 * Free `ptr` from `arena`. `ptr` must be the result of the last allocation
 * from `hgl_stack_alloc`.
 */
void hgl_stack_free(HglArena *arena, void *ptr);

#endif /* HGL_STACK_ALLOC_H */

/*--- Public variables ------------------------------------------------------------------*/

/*--- Stack Alloc functions -------------------------------------------------------------*/

#ifdef HGL_STACK_ALLOC_IMPLEMENTATION

void *hgl_stack_alloc(HglArena *arena, size_t alloc_size)
{
    void *ptr = arena->memory + arena->head;
    size_t size_with_footer = alloc_size + sizeof(HglStackFooter);
    size_t aligned_size     = (size_with_footer + HGL_ARENA_ALIGNMENT - 1) 
                              & ~(HGL_ARENA_ALIGNMENT - 1);

    /* Allocation too small: Return NULL */
    if (alloc_size == 0) {
        return NULL;
    }

    /* Allocation too big: Return NULL */
    if (arena->head + aligned_size > arena->size) {
#ifdef HGL_STACK_ALLOC_DEBUG_PRINTS
        printf("Stack alloc failed. Requested %lu bytes (including footer + padding),"
               "but only %lu remain in arena.\n", aligned_size, arena->size - arena->head);
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */
        return NULL;
    }

    /* Write footer */
    HglStackFooter footer = {
        .alloc_offset = arena->head
    };
    size_t footer_offset = arena->head + aligned_size - sizeof(HglStackFooter);
    memcpy(&arena->memory[footer_offset], &footer, sizeof(HglStackFooter));
    
    /* Update head */
    arena->head += aligned_size;

#ifdef HGL_STACK_ALLOC_DEBUG_PRINTS
    printf("Arena usage: %f%% of %lu KiB\n", 
           100.f * ((float) arena->head / (float) arena->size),
           (arena->size / 1024));
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */

    /* return ptr to chunk */
    return ptr;
}

void hgl_stack_free(HglArena *arena, void *ptr)
{
    HglStackFooter *last_footer = (HglStackFooter *) &arena->memory[arena->head - sizeof(HglStackFooter)];

    /* ptr was not the last thing alloced */
    if (ptr != arena->memory + last_footer->alloc_offset) {
        fprintf(stderr, "hgl_stack_free(): invalid pointer.\n");
        abort();
    }
    
    arena->head = last_footer->alloc_offset;
}

#endif /* HGL_STACK_ALLOC_IMPLEMENTATION */
