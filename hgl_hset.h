
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
 * hgl_hset.h implements a simple to use robin hood hash set. 
 *
 *
 * USAGE:
 *
 * Include hgl_htable.h file like this:
 *
 *     #define HGL_HSET_IMPLEMENTATION
 *     #include "hgl_hset.h"
 *
 *
 * Example:
 *
 *     TODO.. See the examples/ directory or the tests in the test/ directory for now.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_HSET_H
#define HGL_HSET_H

/*--- robin hood hash table-specific macros ---------------------------------------------*/

/* CONFIGURABLE: HGL_HSET_ALLOC, HGL_HSET_FREE */
#if !defined(HGL_HSET_ALLOC) && \
    !defined(HGL_HSET_FREE)
#include <stdlib.h>
#define HGL_HSET_ALLOC  malloc
#define HGL_HSET_FREE   free
#endif

/* CONFIGURABLE: HGL_HSET_LOAD_FACTOR_THRESH */
#ifndef HGL_HSET_LOAD_FACTOR_THRESH
#define HGL_HSET_LOAD_FACTOR_THRESH 0.5f
#endif

/* CONFIGURABLE: HGL_HSET_HASH_SEED */
#ifndef HGL_HSET_HASH_SEED
#define HGL_HSET_HASH_SEED 0x1337
#endif

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    struct HglHashSetBucket *buckets;
    uint32_t n_occupied_buckets;
    uint32_t capacity;
    bool growable;
    void   (*hash)(const void *, size_t, uint32_t, uint32_t *);
    size_t (*keysize)(const void *);
    int    (*keycmp)(const void *, const void *);
} HglHashSet;

static inline HglHashSet hgl_hset_make(void (*hash_func)(const void *, size_t, uint32_t, uint32_t *),
                                       size_t (*keysize_func)(const void *),
                                       int (*keycmp_func)(const void *, const void *),
                                       uint32_t initial_capacity,
                                       bool growable);
static inline void hgl_hset_insert(HglHashSet *hset, void *key);
static inline void hgl_hset_join(HglHashSet *hset, HglHashSet *other);
static inline bool hgl_hset_contains(HglHashSet *hset, void *key);
static inline void hgl_hset_remove(HglHashSet *hset, void *key);
static inline void hgl_hset_destroy(HglHashSet *hset);

#endif

#ifdef HGL_HSET_IMPLEMENTATION

#include <stdio.h> // fprintf
#include <assert.h>

typedef struct HglHashSetBucket
{
    void *key;
    int8_t psl;
} HglHashSetBucket;

static inline void hgl_hset_grow_and_rehash_(HglHashSet *hset)
{
    assert((hset->capacity & 0x80000000) == 0);

    /* construct a new table of double the previous size */
    hset->capacity <<= 1;
    HglHashSetBucket *old_table = hset->buckets;
    hset->buckets = HGL_HSET_ALLOC(hset->capacity * sizeof(HglHashSetBucket));
    hset->n_occupied_buckets = 0;
    if (hset->buckets == NULL) {
        fprintf(stderr, "[hgl_hset_grow_and_rehash_] Error: Allocation of new table failed. Aborting...\n");
        abort();
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < hset->capacity; i++) {
        hset->buckets[i].psl = -1;
    }

    /* iterate old hash table, rehash keys, and put into new table */
    uint32_t old_table_size = hset->capacity >> 1;
    for (uint32_t i = 0; i < old_table_size; i++) {

        HglHashSetBucket *old_bucket = &old_table[i];

        /* Bucket unoccupied: skip */
        if (old_bucket->psl == -1) {
            continue;
        }

        hgl_hset_insert(hset, old_bucket->key);
    }

    /* free old table */
    HGL_HSET_FREE(old_table);
}

/*--- Channel functions -----------------------------------------------------------------*/

static inline HglHashSet hgl_hset_make(void (*hash_func)(const void *, size_t, uint32_t, uint32_t *),
                                       size_t (*keysize_func)(const void *),
                                       int (*keycmp_func)(const void *, const void *),
                                       uint32_t initial_capacity,
                                       bool growable)
{

    if (hash_func == NULL) {
        fprintf(stderr, "[hgl_hset_make] Error: `hash_func` must not be NULL.\n");
    }
    if (keysize_func == NULL) {
        fprintf(stderr, "[hgl_hset_make] Error: `keysize_func` must not be NULL.\n");
    }
    if (keycmp_func == NULL) {
        fprintf(stderr, "[hgl_hset_make] Error: `keycmp_func` must not be NULL.\n");
    }
    if ((initial_capacity & (initial_capacity - 1)) != 0) {
        fprintf(stderr, "[hgl_hset_make] Error: `initial_capacity` must be a power of 2.\n");
    }

    HglHashSet hset = {
        .buckets            = HGL_HSET_ALLOC(initial_capacity * sizeof(HglHashSetBucket)),
        .n_occupied_buckets = 0,
        .capacity           = initial_capacity,
        .growable           = growable,
        .hash               = hash_func,
        .keysize            = keysize_func,
        .keycmp             = keycmp_func,
    };

    if (hset.buckets == NULL) {
        fprintf(stderr, "Allocation failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
        return hset;
    }

    /* init all psls to -1 (free slot) */
    for (uint32_t i = 0; i < hset.capacity; i++) {
        hset.buckets[i].psl = -1;
    }

    return hset;
}

static inline void hgl_hset_destroy(HglHashSet *hset)
{
    HGL_HSET_FREE(hset->buckets);
}

static inline void hgl_hset_insert(HglHashSet *hset, void *key)
{
    /* rehash if needed */
    const float EPSILON = 0.0001f;
    float load_factor = (float) hset->n_occupied_buckets / (float) hset->capacity;
    if (hset->growable && (load_factor > (HGL_HSET_LOAD_FACTOR_THRESH - EPSILON))) {
        hgl_hset_grow_and_rehash_(hset);
    }

    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    hset->hash(key, hset->keysize(key), HGL_HSET_HASH_SEED, &hash);
    index = hash & (hset->capacity - 1);

    /* construct bucket */
    HglHashSetBucket temp;
    HglHashSetBucket bucket_to_insert = {
        .psl   = 0,
        .key   = key,
    };

    /* attempt to insert bucket */
    for (uint32_t i = 0; i < hset->capacity; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashSetBucket *bucket_in_tbl = &hset->buckets[index];

        /* bucket is free: insert */
        if (bucket_in_tbl->psl == -1) {
            *bucket_in_tbl = bucket_to_insert;
            hset->n_occupied_buckets++;
            return;
        }

        /* key collision: do nothing */
        if (hset->keycmp(bucket_in_tbl->key, bucket_to_insert.key) == 0) {
            return;
        }

        /* take from the rich, i.e. swap buckets */
        if (bucket_in_tbl->psl < bucket_to_insert.psl) {
            temp = bucket_to_insert;
            bucket_to_insert = *bucket_in_tbl;
            *bucket_in_tbl = temp;
        }

        /* go to next slot and increment PSL */
        index++;
        index &= (hset->capacity - 1);
        bucket_to_insert.psl++;
    }

    fprintf(stderr, "[hgl_hset_insert] Hash set is full. Aborting...\n");
    abort();
}

static inline void hgl_hset_join(HglHashSet *hset, HglHashSet *other)
{
    for (uint32_t i = 0; i < other->capacity; i++) {
        HglHashSetBucket *bucket = &other->buckets[i];
        if (bucket->psl != -1) {
            hgl_hset_insert(hset, bucket->key);
        }
    }
}

static inline bool hgl_hset_contains(HglHashSet *hset, void *key)
{
    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    hset->hash(key, hset->keysize(key), HGL_HSET_HASH_SEED, &hash);
    index = hash & (hset->capacity - 1);

    /* perform lookup */
    for (int8_t i = 0; i < 127; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashSetBucket *bucket = &hset->buckets[index];

        /* bucket is free or bucket has PSL higher than what the sought after key
         * would have in that slot: no key exists. return NULL */
        if (bucket->psl == -1 || bucket->psl < i) {
            return false;
        }

        /* key found: return it */
        if (hset->keycmp(key, bucket->key) == 0) {
            return true;
        }

        /* go to next slot */
        index++;
        index &= (hset->capacity - 1);
    }

    return false;
}

static inline void hgl_hset_remove(HglHashSet *hset, void *key)
{
    /* compute expected index of key */
    uint32_t hash;
    uint32_t index;
    hset->hash(key, hset->keysize(key), HGL_HSET_HASH_SEED, &hash);
    index = hash & (hset->capacity - 1);

    //printf("Removing \"%s\"\n", (char *) key);

    /* perform lookup */
    for (int8_t i = 0; i < 127; i++) {

        // TODO make max probe sequence length smaller and rehash if we go over the limit.
        // Do benchmarks to find a good limit.

        /* get the bucket at `index` in the table */
        HglHashSetBucket *bucket_in_tbl = &hset->buckets[index];

        /* bucket is free or bucket has PSL higher than what the sought after key
         * would have in that slot: no key exists. return */
        if (bucket_in_tbl->psl == -1 || bucket_in_tbl->psl < i) {
            return;
        }

        /* key found: remove it and backward shift nearby buckets if possible */
        if (hset->keycmp(key, bucket_in_tbl->key) == 0) {
            /* set as unoccupied */
            bucket_in_tbl->psl = -1;

            /* backward shift until empty slot or bucket with PSL = 0 */
            for (uint32_t j = 0; j < hset->capacity; j++) {
                HglHashSetBucket *next_bucket_in_tbl = &hset->buckets[(index + j + 1) & (hset->capacity - 1)];

                /* next slot is empty (psl = -1) or contains bucket with optimal position (psl = 0): return */
                if (next_bucket_in_tbl->psl < 1) {
                    hset->n_occupied_buckets--;
                    return;
                }

                /* next slot contains bucket 1 or more removed from it's optimal position: shift it back */
                *bucket_in_tbl = *next_bucket_in_tbl;
                bucket_in_tbl->psl--;
                next_bucket_in_tbl->psl = -1;

                /* step forward */
                bucket_in_tbl = next_bucket_in_tbl;
            }

            /* should never happen */
            assert(0 && "Bug in hgl_hset_remove");
        }

        /* go to next slot */
        index++;
        index &= (hset->capacity - 1);
    }

    return;
}

#endif

// TODO set join/union

