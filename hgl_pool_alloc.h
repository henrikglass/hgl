
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
 * hgl_pool_alloc.h implements a simple to use pool allocator.
 *
 *
 * USAGE:
 *
 * Include hgl_pool_alloc.h file like this:
 *
 *     #define HGL_POOL_ALLOC_IMPLEMENTATION 
 *     #include "hgl_pool_alloc.h"
 *
 * This will create the implementation of hgl_pool_alloc in the current compilation unit. To
 * include hgl_pool_alloc.h without creating the implementation, simply ommit the #define 
 * of HGL_POOL_ALLOC_IMPLEMENTATION. 
 *
 * Below is a complet listing of the API:
 *
 * HglPool hgl_make_pool(size_t n_chunks, size_t chunk_size)
 * void *hgl_pool_alloc(HglPool *pool)
 * void hgl_pool_free(HglPool *pool, void *ptr)
 * void hgl_pool_free_all(HglPool *pool)
 * void hgl_destroy_pool(HglPool *pool)
 *
 * hgl_pool_alloc allows the user to define the alignment of the pool memory by redefining 
 * HGL_POOL_ALIGNMENT, as such:
 *
 *     #define HGL_POOL_ALIGNMENT 1024
 *
 * The default alignment is 16.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_POOL_ALLOC_H
#define HGL_POOL_ALLOC_H

/*--- Pool alloc-specific macros --------------------------------------------------------*/

#define HGL_POOL_ALLOC_DEBUG_PRINTS

#ifndef HGL_POOL_ALIGNMENT
#define HGL_POOL_ALIGNMENT 16
#endif

/*--- Include files ---------------------------------------------------------------------*/
        
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h> // memset

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    uint8_t *memory;
    void **free_stack;
    ssize_t free_stack_head;
    size_t n_chunks;
    size_t chunk_size;
} HglPool;

/*--- Public function prototypes  -------------------------------------------------------*/

/**
 * Create an pool of capable of holding `n_chunks` elements of size `chunk_size`.
 */
HglPool hgl_make_pool(size_t n_chunks, size_t chunk_size);

/**
 * Allocate a chunk from `pool`.
 */
void *hgl_pool_alloc(HglPool *pool);

/**
 * Free `ptr` from `pool`. `ptr` must be the result of an allocation
 * from `hgl_pool_alloc`.
 */
void hgl_pool_free(HglPool *pool, void *ptr);

/**
 * Free all allocations in `pool`.
 */
void hgl_pool_free_all(HglPool *pool);

/**
 * Destroy the pool (Free the pool itself).
 */
void hgl_destroy_pool(HglPool *pool);

#endif /* HGL_POOL_ALLOC_H */

/*--- Pool Alloc functions -------------------------------------------------------------*/

#ifdef HGL_POOL_ALLOC_IMPLEMENTATION

HglPool hgl_make_pool(size_t n_chunks, size_t chunk_size)
{
    HglPool pool;
    assert(n_chunks != 0 && "Invalid parameters to hgl_make_pool (n_chunks == 0).\n");
    assert(chunk_size != 0 && "Invalid parameters to hgl_make_pool (chunk_size == 0).\n");

    pool.n_chunks        = n_chunks;
    pool.chunk_size      = chunk_size;
    pool.free_stack_head = n_chunks - 1;
    pool.free_stack      = malloc(n_chunks * sizeof(void *));
    pool.memory          = aligned_alloc(HGL_POOL_ALIGNMENT, n_chunks * chunk_size);

    /* malloc failed */
    if (pool.free_stack == NULL || pool.memory == NULL) {
        assert(0 && "Malloc failed");
        return pool;
    }

    /* init (reset) free stack */
    hgl_pool_free_all(&pool);

    return pool;
}

void *hgl_pool_alloc(HglPool *pool)
{
    /* No free chunks: return NULL */
    if (pool->free_stack_head < 0) {
        return NULL;
    }

    /* Pop free chunk from stack */
    return pool->free_stack[pool->free_stack_head--];
}

void hgl_pool_free(HglPool *pool, void *ptr)
{
    uint8_t *ptr8 = (uint8_t *) ptr;

    /* Invalid ptr */
    if ((ptr8 < pool->memory) || 
        (ptr8 > pool->memory + (pool->n_chunks - 1) * pool->chunk_size)) {
        assert(0 && "Freeing invalid ptr (not in range of pool addresses).\n");
        return;
    }

    /* Nothing to free */
    if (pool->free_stack_head > (ssize_t)pool->n_chunks - 1) {
        assert(0 && "Freeing invalid ptr (No unfreed chunks in pool).\n");
        return;
    }

    /* Push free chunk onto stack */
    pool->free_stack[++(pool->free_stack_head)] = ptr;

    return;
}

void hgl_pool_free_all(HglPool *pool)
{
    /* reset free stack */
    int idx = 0;
    pool->free_stack_head = pool->n_chunks - 1;
    for (int i = pool->free_stack_head; i >= 0; i--) {
        pool->free_stack[idx++] = pool->memory + (i * pool->chunk_size);  
    }
}

void hgl_destroy_pool(HglPool *pool)
{
    free(pool->memory);
    free(pool->free_stack);
}

#endif /* HGL_POOL_ALLOC_IMPLEMENTATION */

